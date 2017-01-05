/**
 ******************************************************************************
 *
 * @file       smartsavebutton.cpp
 *
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectWidgetUtils Plugin
 * @{
 * @brief Utility plugin for UAVObject to Widget relation management
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */
#include "smartsavebutton.h"

smartSaveButton::smartSaveButton()
{

}

/**
 * @brief smartSaveButton::addButtons
 * Called only by the ConfigTaskWidget when adding the smart save buttons, depending
 * on whether we want Apply, Save, or Apply & Save.
 * @param save
 * @param apply
 */
void smartSaveButton::addButtons(QPushButton *save, QPushButton *apply)
{
    buttonList.insert(save,save_button);
    buttonList.insert(apply,apply_button);
    connect(save,SIGNAL(clicked()),this,SLOT(processClick()));
    connect(apply,SIGNAL(clicked()),this,SLOT(processClick()));
}

/**
 * @brief smartSaveButton::addApplyButton
 * Called only by the ConfigTaskWidget when adding the smart save buttons, depending
 * on whether we want Apply, Save, or Apply & Save.
 * @param apply
 */
void smartSaveButton::addApplyButton(QPushButton *apply)
{
    buttonList.insert(apply,apply_button);
    connect(apply,SIGNAL(clicked()),this,SLOT(processClick()));
}

/**
 * @brief smartSaveButton::addSaveButton
 * Called only by the ConfigTaskWidget when adding the smart save buttons, depending
 * on whether we want Apply, Save, or Apply & Save.
 * @param save
 */
void smartSaveButton::addSaveButton(QPushButton *save)
{
    buttonList.insert(save,save_button);
    connect(save,SIGNAL(clicked()),this,SLOT(processClick()));
}

/**
 * @brief smartSaveButton::processClick
 */
void smartSaveButton::processClick()
{
    emit beginOp();
    bool save=false;
    QPushButton *button=qobject_cast<QPushButton *>(sender());
    if(!button)
        return;
    if(buttonList.value(button)==save_button)
        save=true;
    processOperation(button,save);

}

/**
 * @brief smartSaveButton::processOperation
 * This is where actual operation processing takes place.
 * @param button
 * @param save is true if we want to issue a saveObjectToFlash request after sending it to the
 *             remote side
 */
void smartSaveButton::processOperation(QPushButton * button,bool save)
{
    emit preProcessOperations();
    QStringList failedUploads;
    QStringList failedSaves;
    QStringList missingObjects;

    if (button) {
        button->setEnabled(false);
        button->setIcon(QIcon(":/uploader/images/system-run.svg"));
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    bool error = false;
    for (auto o : objects) {
        auto obj = o.toStrongRef();
        if (!obj) {
            Q_ASSERT(false);
            qWarning() << "Invalid object!";
            error = true;
            break;
        }

        // We only allow to send/save Settings objects using this method
        if (!obj->isSettings()) {
            qDebug() << "[smartsavebutton.cpp] Error, tried to apply/save a non-settings object: " << obj->getName();
            continue;
        }

        if (!obj->getIsPresentOnHardware()) {
            if(mandatoryList.value(obj.data(), true))
                missingObjects.append(obj->getName());
            continue;
        }

        UAVObject::Metadata mdata = obj->getMetadata();
        if (UAVObject::getGcsAccess(mdata) == UAVObject::ACCESS_READONLY) {
            if(mandatoryList.value(obj.data(), true))
                failedUploads.append(obj->getName());
            continue;
        }

        bool upload_result = false;
        qDebug() << "[smartsavebutton.cpp] Sending object to remote end - " << obj->getName();
        auto conn = connect(obj.data(), static_cast<void (UAVObject::*)(QSharedPointer<UAVObject>, bool)>(&UAVObject::transactionCompleted),
                this, [&loop, &upload_result, obj] (QSharedPointer<UAVObject> rxObj, bool result) {
            if (obj == rxObj) {
                upload_result = result;
                loop.quit();
            }
        });
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        obj->updated();
        // Three things can happen now:
        // - If object is ACK'ed then we'll get a transactionCompleted signal once ACK arrives
        //   with a success value at 'true'
        // - If remote side does not know it, or object is not ACK'ed then we'll get a timeout, which we catch below
        // - If object is ACK'ed and message gets lost, we'll get a transactionCompleted with a
        //   success value at 'false'
        //
        // Note: settings objects should always be ACK'ed, so a timeout should always be because of a lost
        //       message over the telemetry link.
        //
        // Note 2: the telemetry link does max 3 tries with 250ms interval when sending an object
        //         update over the link
        timer.start(1000);
        loop.exec();
        disconnect(conn);
        if (!upload_result && !timer.isActive())
            qWarning() << "[smartsavebutton.cpp] Upload timeout for object" << obj->getName();
        timer.stop();
        disconnect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        if (!upload_result) {
            qWarning() << "[smartsavebutton.cpp] Object upload error:" << obj->getName();
            if (mandatoryList.value(obj.data(), true))
                failedUploads.append(obj->getName());
            continue;
        }

        // Now object is uploaded, we can move on to saving it to flash:
        if (save) {
            qDebug() << "[smartsavebutton.cpp] Save request for object" << obj->getName();

            bool save_result = false;
            auto objId = obj->getObjID();

            auto conn = connect(utilMngr, &UAVObjectUtilManager::saveCompleted,
                    this, [&loop, &save_result, objId] (quint32 rxObjId, bool status) {
                // The saveOjectToFlash method manages its own save queue, so we can be
                // in a situation where we get a saving_finished message for an object
                // which is not the one we're interested in, hence the check below:
                if (rxObjId == objId) {
                    save_result = status;
                    loop.quit();
                }
            });
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            utilMngr->saveObjectToFlash(obj);
            // Now, here is what will happen:
            // - saveObjectToFlash is going to attempt the save operation
            // - If save succeeds, then it will issue a saveCompleted signal with the ObjectID and 'true'
            // - If save fails, either because of error or timeout, then it will issue a saveCompleted signal
            //   with the ObjectID and 'false'.
            //
            // Note: in case of link timeout, the telemetry layer will retry up to 2 times, we don't
            // need to retry ourselves here.
            //
            // Note 2: saveObjectToFlash manages save operations in a queue, so there is no guarantee that
            // the first "saveCompleted" signal is for the object we just asked to save.
            timer.start(2000);
            loop.exec();
            disconnect(conn);
            if (!save_result && !timer.isActive())
                qDebug() << "[smartsavebutton.cpp] Saving timeout for object" << obj->getName();
            timer.stop();
            disconnect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

            if (!save_result) {
                qDebug() << "[smartsavebutton.cpp] failed to save:" << obj->getName();
                if (mandatoryList.value(obj.data(), true))
                    failedSaves.append(obj->getName());
            }
        }
    }

    QString result;
    if (!failedSaves.isEmpty()) {
        error = true;
        result.append("Objects not saved:\n");
        foreach (QString str, failedSaves) {
            result.append(str + "\n");
        }
    }
    if (!failedUploads.isEmpty()) {
        error = true;
        if(!result.isEmpty())
            result.append("\n");
        result.append("Objects not uploaded:\n");
        foreach (QString str, failedUploads) {
            result.append(str + "\n");
        }
    }
    if (!missingObjects.isEmpty()) {
        error = true;
        if(!result.isEmpty())
            result.append("\n");
        result.append("Objects not present on the hardware:\n");
        foreach (QString str, missingObjects) {
            result.append(str + "\n");
        }
    }

    if (!error) {
        result = "All operations finished successfully";
        if (button)
            button->setIcon(QIcon(":/uploader/images/dialog-apply.svg"));
        emit saveSuccessfull();
    } else {
        if(button) {
            if(!failedSaves.isEmpty() || !failedUploads.isEmpty())
                button->setIcon(QIcon(":/uploader/images/process-stop.svg"));
            else
                button->setIcon(QIcon(":/uploader/images/warning.svg"));
        }
    }

    qDebug()<<"RESULT"<<result<<missingObjects<<failedSaves<<failedUploads;
    if (button) {
        button->setToolTip(result); // TODO: perhaps give the user better feedback about what actually happened?
        button->setEnabled(true);
    }

    emit endOp();
}

/**
 * @brief smartSaveButton::setObjects
 * Sets all monitored objects in one operation
 * @param list
 */
void smartSaveButton::setObjects(QList<QWeakPointer<UAVDataObject>> list)
{
    objects = list;
}

/**
 * @brief smartSaveButton::addObject
 * The smartSaveButton contains a list of objects it will work with, addObject
 * is used to add a new object to a smartSaveButton instance.
 * @param obj
 */
void smartSaveButton::addObject(QWeakPointer<UAVDataObject> obj)
{
    if (!obj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object!";
        return;
    }
    if (!objects.contains(obj))
        objects.append(obj);
}

/**
 * @brief smartSaveButton::addObject
 * The smartSaveButton contains a list of objects it will work with, addObject
 * is used to add a new object to a smartSaveButton instance.
 * @param obj object to add to the framework
 * @param isMandatory if object is not mandatory the save or upload result
 * will show as successfull even if the object doesn't exist on the hardware
 */
void smartSaveButton::addObject(QWeakPointer<UAVDataObject> obj, bool isMandatory)
{
    if (!obj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object!";
        return;
    }
    if (!objects.contains(obj))
        objects.append(obj);
    if (!isMandatory)
        mandatoryList.insert(obj.data(), false); // we use default value of true on retrieve
}

/**
 * @brief smartSaveButton::removeObject
 * The smartSaveButton contains a list of objects it will work with, addObject
 * is used to remove an object from a smartSaveButton instance.
 * @param obj
 */
void smartSaveButton::removeObject(QWeakPointer<UAVDataObject> obj)
{
    if (objects.contains(obj))
        objects.removeAll(obj);
}

/**
 * @brief smartSaveButton::removeAllObjects
 * Remove all tracked objects at once.
 */
void smartSaveButton::removeAllObjects()
{
    objects.clear();
}

void smartSaveButton::clearObjects()
{
    objects.clear();
}

void smartSaveButton::enableControls(bool value)
{
    foreach(QPushButton * button,buttonList.keys())
        button->setEnabled(value);
}

void smartSaveButton::resetIcons()
{
    foreach(QPushButton * button,buttonList.keys()) {
        button->setToolTip("");
        button->setIcon(QIcon());
    }
}

/**
 * Set a UAVObject as not mandatory, meaning that if it doesn't exist on the 
 * hardware a failed upload or save will be marked as successfull
 */
void smartSaveButton::setNotMandatory(QWeakPointer<UAVDataObject> obj)
{
    mandatoryList.insert(obj.data(), false);
}

void smartSaveButton::apply()
{
    processOperation(NULL, false);
}

void smartSaveButton::save()
{
    processOperation(NULL, true);
}


