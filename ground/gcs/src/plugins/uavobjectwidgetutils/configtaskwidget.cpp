/**
 ******************************************************************************
 *
 * @file       configtaskwidget.cpp
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
#include "configtaskwidget.h"
#include "connectiondiagram.h"
#include <QWidget>
#include <QLineEdit>
#include "uavsettingsimportexport/uavsettingsimportexportmanager.h"
#include <coreplugin/connectionmanager.h>
#include <coreplugin/icore.h>

/**
 * Constructor
 */
ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent),currentBoard(0),isConnected(false),allowWidgetUpdates(true),smartsave(NULL),dirty(false),outOfLimitsStyle("background-color: rgb(255, 180, 0);"),timeOut(NULL)
{
    pm = ExtensionSystem::PluginManager::instance();
    objManager = pm->getObject<UAVObjectManager>();
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    utilMngr = pm->getObject<UAVObjectUtilManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(connected()), this, SIGNAL(autoPilotConnected()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SIGNAL(autoPilotDisconnected()),Qt::UniqueConnection);
    UAVSettingsImportExportManager * importexportplugin =  pm->getObject<UAVSettingsImportExportManager>();
    connect(importexportplugin,SIGNAL(importAboutToBegin()),this,SLOT(invalidateObjects()));
}

/**
 * Add a widget to the dirty detection pool
 * @param widget to add to the detection pool
 */
void ConfigTaskWidget::addWidget(QWidget * widget)
{
    addUAVObjectToWidgetRelation("","",widget);
}
/**
 * Add an object to the management system
 * @param objectName name of the object to add to the management system
 */
void ConfigTaskWidget::addUAVObject(QString objectName,QList<int> * reloadGroups)
{
    addUAVObjectToWidgetRelation(objectName, "", NULL, 0, 1, false, false, reloadGroups);
}

void ConfigTaskWidget::addUAVObject(QSharedPointer<UAVObject> object, QList<int> *reloadGroups)
{
    QString objstr;
    if (object)
        objstr = object->getName();
    addUAVObject(objstr, reloadGroups);

}
/**
 * Add an UAVObject field to widget relation to the management system
 * @param object name of the object to add
 * @param field name of the field to add
 * @param widget pointer to the widget whitch will display/define the field value
 * @param index index of the field element to add to this relation
 */
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString index)
{
    auto obj = objManager->getObject(object);
    if (!obj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object! " << object;
        return;
    }
    auto _field = obj->getField(field);
    if (!_field) {
        Q_ASSERT(false);
        qWarning() << "Invalid object field! " << field;
        return;
    }
    addUAVObjectToWidgetRelation(object, field, widget, _field->getElementNames().indexOf(index));
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(QSharedPointer<UAVObject> obj, QSharedPointer<UAVObjectField> field, QWidget *widget, QString index)
{
    Q_ASSERT(obj && field);
    QString objstr;
    QString fieldstr;
    if (obj)
        objstr = obj->getName();
    if (field)
        fieldstr = field->getName();
    addUAVObjectToWidgetRelation(objstr, fieldstr, widget, index);
}
/**
 * Add a UAVObject field to widget relation to the management system
 * @param object name of the object to add
 * @param field name of the field to add
 * @param widget pointer to the widget whitch will display/define the field value
 * @param element name of the element of the field element to add to this relation
 * @param scale scale value of this relation
 * @param isLimited bool to signal if this widget contents is limited in value
 * @param defaultReloadGroups default and reload groups this relation belongs to
 * @param instID instance ID of the object used on this relation
 */
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString element, double scale, bool isLimited, bool useUnits, QList<int> *defaultReloadGroups, quint32 instID)
{
    auto obj = objManager->getObject(object, instID);
    if (!obj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object! " << object;
        return;
    }
    int index = 0;
    if (!field.isEmpty() && !element.isEmpty()) {
        auto _field = obj->getField(field);
        if (!_field) {
            Q_ASSERT(false);
            qWarning() << "Invalid object field! " << field;
            return;
        }
        index= _field->getElementNames().indexOf(QString(element));
    }
    addUAVObjectToWidgetRelation(object, field, widget, index, scale, isLimited, useUnits, defaultReloadGroups, instID);
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(QSharedPointer<UAVObject> obj, QSharedPointer<UAVObjectField> field, QWidget *widget, QString element, double scale, bool isLimited, bool useUnits, QList<int> *defaultReloadGroups, quint32 instID)
{
    Q_ASSERT(obj && field);
    QString objstr;
    QString fieldstr;
    if (obj)
        objstr = obj->getName();
    if (field)
        fieldstr = field->getName();
    addUAVObjectToWidgetRelation(objstr, fieldstr, widget, element, scale, isLimited, useUnits, defaultReloadGroups, instID);
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(QSharedPointer<UAVObject> obj, QSharedPointer<UAVObjectField> field, QWidget *widget, int index, double scale, bool isLimited, bool useUnits, QList<int>* defaultReloadGroups, quint32 instID)
{
    Q_ASSERT(obj && field);
    QString objstr;
    QString fieldstr;
    if (obj)
        objstr = obj->getName();
    if (field)
        fieldstr = field->getName();
    addUAVObjectToWidgetRelation(objstr, fieldstr, widget, index, scale, isLimited, useUnits, defaultReloadGroups, instID);
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, int index, double scale, bool isLimited, bool useUnits, QList<int> *defaultReloadGroups, quint32 instID)
{
    if(addShadowWidget(object, field, widget, index, scale, isLimited, useUnits, defaultReloadGroups, instID))
        return;

    auto obj = QSharedPointer<UAVObject>();
    auto _field = QSharedPointer<UAVObjectField>();
    if (!object.isEmpty()) {
        obj = objManager->getObject(object, instID);
        if (!obj) {
            Q_ASSERT(false);
            qWarning() << "Invalid object! " << object;
            return;
        }
        objectUpdates.insert(obj.data(), true);
        connect(obj.data(), &UAVObject::objectUpdated, this, &ConfigTaskWidget::objectUpdated, Qt::UniqueConnection);
        connect(obj.data(), &UAVObject::objectUpdated, this, &ConfigTaskWidget::refreshWidgetsValues, Qt::UniqueConnection);
        auto dobj = obj.dynamicCast<UAVDataObject>();
        if (dobj) {
            connect(dobj.data(), &UAVDataObject::presentOnHardwareChanged, this,
                    &ConfigTaskWidget::doRefreshHiddenObjects, Qt::UniqueConnection);
            if (widget)
                setWidgetEnabledByObj(widget, dobj->getIsPresentOnHardware());
        }
    }

    if(!field.isEmpty() && obj)
        _field = obj->getField(field);

    objectToWidget *ow = new objectToWidget();
    ow->field = _field;
    ow->object = obj;
    ow->widget = widget;
    ow->index = index;
    ow->scale = scale;
    ow->isLimited = isLimited;
    ow->useUnits = useUnits;
    objOfInterest.append(ow);

    if (obj) {
        if (smartsave)
            smartsave->addObject(obj.dynamicCast<UAVDataObject>());
    }

    if (!ow->widget) {
        if (defaultReloadGroups && obj) {
            for (int i : *defaultReloadGroups) {
                if (this->defaultReloadGroups.contains(i)) {
                    this->defaultReloadGroups.value(i)->append(ow);
                } else {
                    this->defaultReloadGroups.insert(i, new QList<objectToWidget *>());
                    this->defaultReloadGroups.value(i)->append(ow);
                }
            }
        }
    } else {
        connectWidgetUpdatesToSlot(widget,SLOT(widgetsContentsChanged()));
        if(defaultReloadGroups)
            addWidgetToDefaultReloadGroups(widget,defaultReloadGroups);
        shadowsList.insert(widget,ow);
        loadWidgetLimits(widget, _field, index, isLimited, useUnits, scale);
    }
}

/**
 * Set a UAVObject as not mandatory, meaning that if it doesn't exist on the
 * hardware a failed upload or save will be marked as successfull
 */
void ConfigTaskWidget::setNotMandatory(QString object)
{
    auto obj = objManager->getObject(object);
    if (!obj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object! " << object;
        return;
    }
    if(smartsave)
        smartsave->setNotMandatory(obj.dynamicCast<UAVDataObject>());
}


/**
 * destructor
 */
ConfigTaskWidget::~ConfigTaskWidget()
{
    if(smartsave)
        delete smartsave;
    foreach(QList<objectToWidget*>* pointer,defaultReloadGroups.values())
    {
        if(pointer)
            delete pointer;
    }
    foreach (objectToWidget* oTw, objOfInterest)
    {
        if(oTw)
            delete oTw;
    }
    if(timeOut)
    {
        delete timeOut;
        timeOut = NULL;
    }
}

void ConfigTaskWidget::saveObjectToSD(QSharedPointer<UAVObject> obj)
{
    // saveObjectToSD is now handled by the UAVUtils plugin in one
    // central place (and one central queue)
    auto utilMngr = getObjectUtilManager();
    if (utilMngr)
        utilMngr->saveObjectToFlash(obj);
    else
        qWarning() << "Failed to save object to flash!";
}


/**
 * @brief ConfigTaskWidget::getObjectManager Utility function to get a pointer to the object manager
 * @return pointer to the UAVObjectManager
 */
UAVObjectManager* ConfigTaskWidget::getObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();
    if (!objMngr) {
        Q_ASSERT(false);
        qWarning() << "Could not get UAVObjectManager!";
    }
    return objMngr;
}


/**
 * @brief ConfigTaskWidget::getObjectUtilManager Utility function to get a pointer to the object manager utilities
 * @return pointer to the UAVObjectUtilManager
 */
UAVObjectUtilManager* ConfigTaskWidget::getObjectUtilManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    Q_ASSERT(utilMngr);
    return utilMngr;
}

// ************************************
// telemetry start/stop connect/disconnect signals

void ConfigTaskWidget::onAutopilotDisconnect()
{
    isConnected=false;
    enableControls(false);
    invalidateObjects();
    setDirty(false);
}

void ConfigTaskWidget::forceConnectedState()//dynamic widgets don't recieve the connected signal. This should be called instead.
{
    isConnected=true;
    setDirty(false);
}

void ConfigTaskWidget::onAutopilotConnect()
{
    if (utilMngr)
        currentBoard = utilMngr->getBoardModel();

    invalidateObjects();
    isConnected=true;
    loadAllLimits();
    enableControls(true);
    refreshWidgetsValues();
    setDirty(false);

    emit autoPilotConnected();
}

void ConfigTaskWidget::loadAllLimits()
{
    foreach(objectToWidget * ow,objOfInterest)
    {
        loadWidgetLimits(ow->widget, ow->field, ow->index, ow->isLimited, ow->useUnits, ow->scale);
    }
}

/**
 * SLOT Function used to populate the widgets with the initial values
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::populateWidgets()
{
    bool dirtyBack = dirty;
    emit populateWidgetsRequested();
    foreach(objectToWidget *ow, objOfInterest) {
        if(ow->object && ow->field && ow->widget)
            setWidgetFromField(ow->widget, ow->field, ow->index, ow->scale, ow->isLimited, ow->useUnits);
    }
    setDirty(dirtyBack);
}
/**
 * SLOT function used to refresh the widgets contents of the widgets with relation to
 * object field added to the framework pool
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::refreshWidgetsValues(QSharedPointer<UAVObject> obj)
{
    if (!allowWidgetUpdates)
        return;

    bool dirtyBack = dirty;
    emit refreshWidgetsValuesRequested();
    foreach(objectToWidget *ow, objOfInterest) {
        if (ow->object && ow->field && ow->widget && (ow->object == obj || !obj))
                setWidgetFromField(ow->widget, ow->field, ow->index, ow->scale, ow->isLimited, ow->useUnits);
    }
    setDirty(dirtyBack);

}
/**
 * SLOT function used to update the uavobject fields from widgets with relation to
 * object field added to the framework pool
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::updateObjectsFromWidgets()
{
    emit updateObjectsFromWidgetsRequested();
    foreach(objectToWidget *ow, objOfInterest) {
        if (ow->object && ow->field)
            setFieldFromWidget(ow->widget, ow->field, ow->index, ow->scale, ow->useUnits);

    }
}
/**
 * SLOT function used handle help button presses
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::helpButtonPressed()
{
    QString url=helpButtonList.value((QPushButton*)sender(),QString());
    if(!url.isEmpty())
        QDesktopServices::openUrl( QUrl(url, QUrl::StrictMode) );
}
/**
 * Add update and save buttons to the form
 * multiple buttons can be added for the same function
 * @param update pointer to the update button
 * @param save pointer to the save button
 */
void ConfigTaskWidget::addApplySaveButtons(QPushButton *update, QPushButton *save)
{
    if(!smartsave)
    {
        smartsave=new smartSaveButton();
        connect(smartsave,SIGNAL(preProcessOperations()), this, SLOT(updateObjectsFromWidgets()));
        connect(smartsave,SIGNAL(saveSuccessfull()),this,SLOT(clearDirty()));
        connect(smartsave,SIGNAL(beginOp()),this,SLOT(disableObjUpdates()));
        connect(smartsave,SIGNAL(endOp()),this,SLOT(enableObjUpdates()));
    }
    if(update && save)
        smartsave->addButtons(save,update);
    else if (update)
        smartsave->addApplyButton(update);
    else if (save)
        smartsave->addSaveButton(save);
    if (objOfInterest.count() > 0) {
        foreach(objectToWidget *oTw, objOfInterest)
            smartsave->addObject(oTw->object.toStrongRef().dynamicCast<UAVDataObject>());
    }
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    if(telMngr->isConnected())
        enableControls(true);
    else
        enableControls(false);
}
/**
 * SLOT function used the enable or disable the SAVE, UPLOAD and RELOAD buttons
 * @param enable set to true to enable the buttons or false to disable them
 * @param field name of the field to add
 */
void ConfigTaskWidget::enableControls(bool enable)
{
    if(smartsave)
        smartsave->enableControls(enable);
    for (QPushButton *button : reloadButtonList)
        setWidgetEnabledByObj(button, enable);
    for (QPushButton *button : rebootButtonList)
        setWidgetEnabledByObj(button, enable);
    for (QPushButton *button : connectionsButtonList)
        setWidgetEnabledByObj(button, enable);
}
/**
 * @brief ConfigTaskWidget::forceShadowUpdates
 */
void ConfigTaskWidget::forceShadowUpdates()
{
    foreach (objectToWidget *oTw, objOfInterest) {
        foreach (shadow *sh, oTw->shadowsList) {
            disconnectWidgetUpdatesToSlot(sh->widget, SLOT(widgetsContentsChanged()));

            checkWidgetsLimits(sh->widget, oTw->field, oTw->index, sh->isLimited, sh->useUnits, getVariantFromWidget(oTw->widget, oTw->scale), sh->scale);

            if (auto field = oTw->field.toStrongRef())
                setWidgetFromVariant(sh->widget, getVariantFromWidget(oTw->widget, oTw->scale), sh->scale, field->getRadix(), sh->useUnits ? field->getUnits() : "");
            else
                qWarning() << "Failed to set shadow widget!";

            emit widgetContentsChanged(sh->widget);
            connectWidgetUpdatesToSlot(sh->widget, SLOT(widgetsContentsChanged()));
        }
    }
}
/**
 * SLOT function called when one of the widgets contents added to the framework changes
 */
void ConfigTaskWidget::widgetsContentsChanged()
{
    auto from = qobject_cast<QWidget *>(sender());
    if (!from) {
        qWarning() << "Spurious signal from non-widget!";
        return;
    }

    emit widgetContentsChanged(from);
    double scale = 1;
    objectToWidget *oTw = shadowsList.value(from, Q_NULLPTR);
    if (oTw) {
        if (oTw->widget == from) {
            scale = oTw->scale;
            checkWidgetsLimits(from, oTw->field, oTw->index, oTw->isLimited, oTw->useUnits, getVariantFromWidget(from, oTw->scale), oTw->scale);
        } else {
            for (const shadow *sh : oTw->shadowsList) {
                if (sh->widget == from) {
                    scale = sh->scale;
                    checkWidgetsLimits(from, oTw->field, oTw->index, sh->isLimited, sh->useUnits, getVariantFromWidget(from, scale), scale);
                }
            }

            disconnectWidgetUpdatesToSlot(oTw->widget, SLOT(widgetsContentsChanged()));
            checkWidgetsLimits(oTw->widget, oTw->field, oTw->index, oTw->isLimited, oTw->useUnits, getVariantFromWidget(from, scale), oTw->scale);

            if (auto field = oTw->field.toStrongRef())
                setWidgetFromVariant(oTw->widget, getVariantFromWidget(from, scale), oTw->scale, field->getRadix(), oTw->useUnits ? field->getUnits() : "");
            else
                qWarning() << "Failed to set shadow widget!";

            emit widgetContentsChanged(oTw->widget);
            connectWidgetUpdatesToSlot(oTw->widget, SLOT(widgetsContentsChanged()));
        }

        for (const shadow *sh : oTw->shadowsList) {
            if (sh->widget!= from) {
                disconnectWidgetUpdatesToSlot(sh->widget, SLOT(widgetsContentsChanged()));
                checkWidgetsLimits(sh->widget, oTw->field, oTw->index, sh->isLimited, sh->useUnits, getVariantFromWidget(from, scale), sh->scale);

                if (auto field = oTw->field.toStrongRef())
                    setWidgetFromVariant(sh->widget, getVariantFromWidget(from, scale), sh->scale, field->getRadix(), sh->useUnits ? field->getUnits() : "");
                else
                    qWarning() << "Failed to set shadow widget!";

                emit widgetContentsChanged(sh->widget);
                connectWidgetUpdatesToSlot(sh->widget, SLOT(widgetsContentsChanged()));
            }
        }
    }
    if (smartsave)
        smartsave->resetIcons();
    setDirty(true);
}
/**
 * SLOT function used clear the forms dirty status flag
 */
void ConfigTaskWidget::clearDirty()
{
    setDirty(false);
}
/**
 * Sets the form's dirty status flag
 * @param value
 */
void ConfigTaskWidget::setDirty(bool value)
{
    dirty=value;
}
/**
 * Checks if the form is dirty (unsaved changes)
 * @return true if the form has unsaved changes
 */
bool ConfigTaskWidget::isDirty()
{
    return dirty;
}
/**
 * @brief ConfigTaskWidget::isAutopilotConnected Checks if the autopilot is connected
 * @return true if an autopilot is connected
 */
bool ConfigTaskWidget::isAutopilotConnected()
{
    return isConnected;
}

/**
 * SLOT function used to disable widget contents changes when related object field changes
 */
void ConfigTaskWidget::disableObjUpdates()
{
    allowWidgetUpdates = false;
    for (objectToWidget *obj : objOfInterest) {
        if (obj->object) // weak ref is okay for disconnect
            disconnect(obj->object.data(), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues(UAVObject*)));
    }
}
/**
 * SLOT function used to enable widget contents changes when related object field changes
 */
void ConfigTaskWidget::enableObjUpdates()
{
    allowWidgetUpdates = true;
    for (objectToWidget *obj : objOfInterest) {
        if (obj->object) // weak ref is okay for connect
            connect(obj->object.data(), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues(UAVObject*)), Qt::UniqueConnection);
    }
}
/**
 * Called when an uav object is updated
 * @param obj pointer to the object whitch has just been updated
 */
void ConfigTaskWidget::objectUpdated(QWeakPointer<UAVObject> obj)
{
    if (obj)
        objectUpdates[obj.data()] = true;
}
/**
 * Checks if all objects added to the pool have already been updated
 * @return true if all objects added to the pool have already been updated
 */
bool ConfigTaskWidget::allObjectsUpdated()
{
    for (const auto &obj : objectUpdates.keys()) {
        if (!objectUpdates[obj])
            return false;
    }
    return true;
}
/**
 * Adds a new help button
 * @param button pointer to the help button
 * @param url url to open in the browser when the help button is pressed
 */
void ConfigTaskWidget::addHelpButton(QPushButton *button, QString url)
{
    helpButtonList.insert(button,url);
    connect(button,SIGNAL(clicked()),this,SLOT(helpButtonPressed()));

}
/**
 * Invalidates all the uav objects "is updated" flag
 */
void ConfigTaskWidget::invalidateObjects()
{
    for (const auto &obj : objectUpdates.keys())
        objectUpdates[obj] = false;
}
/**
 * SLOT call this to apply changes to uav objects
 */
void ConfigTaskWidget::apply()
{
    if(smartsave)
        smartsave->apply();
}
/**
 * SLOT call this to save changes to uav objects
 */
void ConfigTaskWidget::save()
{
    if(smartsave)
        smartsave->save();
}
/**
 * Adds a new shadow widget
 * shadow widgets are widgets whitch have a relation to an object already present on the framework pool i.e. already added trough addUAVObjectToWidgetRelation
 * This function doesn't have to be used directly, addUAVObjectToWidgetRelation will call it if a previous relation exhists.
 * @return returns false if the shadow widget relation failed to be added (no previous relation exhisted)
 */
bool ConfigTaskWidget::addShadowWidget(QString objectName, QString fieldName, QWidget *widget, int index, double scale, bool isLimited, bool useUnits, QList<int>* defaultReloadGroups, quint32 instID)
{
    for (objectToWidget *oTw : objOfInterest) {
        auto object = oTw->object.toStrongRef();
        auto field = oTw->field.toStrongRef();
        if (!object || !oTw->widget || !field)
            continue;
        if (object->getName() == objectName && field->getName() == fieldName && oTw->index == index &&
                object->getInstID() == instID) {
            shadow *sh = Q_NULLPTR;
            //prefer anything else to QLabel
            if (qobject_cast<QLabel *>(oTw->widget) && !qobject_cast<QLabel *>(widget)) {
                sh = new shadow;
                sh->isLimited = oTw->isLimited;
                sh->scale = oTw->scale;
                sh->widget = oTw->widget;
                sh->useUnits = oTw->useUnits;
                oTw->isLimited = isLimited;
                oTw->scale = scale;
                oTw->widget = widget;
                oTw->useUnits = useUnits;
            //prefer QDoubleSpinBox to anything else
            } else if(!qobject_cast<QDoubleSpinBox *>(oTw->widget) && qobject_cast<QDoubleSpinBox *>(widget)) {
                sh = new shadow;
                sh->isLimited = oTw->isLimited;
                sh->scale = oTw->scale;
                sh->widget = oTw->widget;
                sh->useUnits = oTw->useUnits;
                oTw->isLimited = isLimited;
                oTw->scale = scale;
                oTw->widget = widget;
                oTw->useUnits = useUnits;
            } else {
                sh = new shadow;
                sh->isLimited = isLimited;
                sh->scale = scale;
                sh->widget = widget;
                sh->useUnits = useUnits;
            }
            shadowsList.insert(widget, oTw);
            oTw->shadowsList.append(sh);

            connectWidgetUpdatesToSlot(widget, SLOT(widgetsContentsChanged()));
            if(defaultReloadGroups)
                addWidgetToDefaultReloadGroups(widget, defaultReloadGroups);
            loadWidgetLimits(widget, field, oTw->index, isLimited, useUnits, scale);
            auto dobj = object.dynamicCast<UAVDataObject>();
            if (dobj) {
                connect(dobj.data(), &UAVDataObject::presentOnHardwareChanged,this,
                        &ConfigTaskWidget::doRefreshHiddenObjects, Qt::UniqueConnection);
                if (widget)
                    setWidgetEnabledByObj(widget, dobj->getIsPresentOnHardware());
            }
            return true;
        }
    }
    return false;
}
/**
 * Auto loads widgets based on the Dynamic property named "objrelation"
 * Check the wiki for more information
 */
void ConfigTaskWidget::autoLoadWidgets()
{
    QPushButton *saveButtonWidget = NULL;
    QPushButton *applyButtonWidget = NULL;
    foreach (QWidget *widget, this->findChildren<QWidget *>()) {
        QVariant info = widget->property("objrelation");
        if (info.isValid()) {
            uiRelationAutomation uiRelation;
            uiRelation.buttonType = none;
            uiRelation.scale = 1;
            uiRelation.element = QString();
            uiRelation.haslimits = false;
            uiRelation.useUnits = false;
            foreach (QString str, info.toStringList()) {
                QString prop = str.split(":").at(0);
                QString value = str.split(":").at(1);
                if (prop == "objname") {
                    uiRelation.objname = value;
                } else if (prop == "fieldname") {
                    uiRelation.fieldname = value;
                } else if (prop=="element") {
                    uiRelation.element = value;
                } else if (prop == "scale") {
                    if (value == "null")
                        uiRelation.scale = 1;
                    else
                        uiRelation.scale = value.toDouble();
                } else if (prop == "haslimits") {
                    if (value == "yes")
                        uiRelation.haslimits = true;
                    else
                        uiRelation.haslimits = false;
                } else if (prop == "button") {
                    if (value == "save")
                        uiRelation.buttonType = save_button;
                    else if (value == "apply")
                        uiRelation.buttonType = apply_button;
                    else if (value == "reload")
                        uiRelation.buttonType = reload_button;
                    else if (value == "default")
                        uiRelation.buttonType = default_button;
                    else if (value == "help")
                        uiRelation.buttonType = help_button;
                    else if (value == "reboot")
                        uiRelation.buttonType = reboot_button;
                    else if (value == "connectiondiagram")
                        uiRelation.buttonType = connections_button;
                } else if (prop == "buttongroup") {
                    foreach (QString s, value.split(","))
                        uiRelation.buttonGroup.append(s.toInt());
                } else if (prop == "url") {
                    uiRelation.url = str.mid(str.indexOf(":") + 1);
                } else if (prop == "checkedoption") {
                    widget->setProperty("checkedOption", value);
                } else if (prop == "uncheckedoption") {
                    widget->setProperty("unCheckedOption", value);
                } else if (prop == "useunits") {
                    if (value == "yes")
                        uiRelation.useUnits = true;
                    else
                        uiRelation.useUnits = false;
                }
            }

            if (uiRelation.buttonType != none) {
                QPushButton *button = NULL;
                switch (uiRelation.buttonType) {
                case save_button:
                    saveButtonWidget = qobject_cast<QPushButton *>(widget);
                    if (saveButtonWidget)
                        addApplySaveButtons(NULL, saveButtonWidget);
                    break;
                case apply_button:
                    applyButtonWidget = qobject_cast<QPushButton *>(widget);
                    if (applyButtonWidget)
                        addApplySaveButtons(applyButtonWidget, NULL);
                    break;
                case default_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button) {
                        if (!uiRelation.buttonGroup.length()) {
                            qWarning() << "[autoLoadWidgets] No button group specified for default button!";
                            uiRelation.buttonGroup.append(0);
                        }
                        addDefaultButton(button, uiRelation.buttonGroup.at(0));
                    }
                    break;
                case reload_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button) {
                        if (!uiRelation.buttonGroup.length()) {
                            qWarning() << "[autoLoadWidgets] No button group specified for reload button!";
                            uiRelation.buttonGroup.append(0);
                        }
                        addReloadButton(button, uiRelation.buttonGroup.at(0));
                    }
                    break;
                case help_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button)
                        addHelpButton(button, uiRelation.url);
                    break;
                case reboot_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button)
                        addRebootButton(button);
                    break;
                case connections_button:
                    button = qobject_cast<QPushButton *>(widget);
                    if (button)
                        addConnectionsButton(button);
                    break;
                default:
                    break;
                }
            } else {
                QWidget *wid = qobject_cast<QWidget *>(widget);
                if (wid) {
                    addUAVObjectToWidgetRelation(uiRelation.objname, uiRelation.fieldname, wid, uiRelation.element,
                                                 uiRelation.scale, uiRelation.haslimits, uiRelation.useUnits, &uiRelation.buttonGroup);
                }
            }
        }
    }
    refreshWidgetsValues();
    forceShadowUpdates();
}
/**
 * Adds a widget to a list of default/reload groups
 * default/reload groups are groups of widgets to be set with default or reloaded (values from persistent memory) when a defined button is pressed
 * @param widget pointer to the widget to be added to the groups
 * @param groups list of the groups on which to add the widget
 */
void ConfigTaskWidget::addWidgetToDefaultReloadGroups(QWidget *widget, QList<int> * groups)
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        bool addOTW=false;
        if(oTw->widget==widget)
            addOTW=true;
        else
        {
            foreach(shadow * sh, oTw->shadowsList)
            {
                if(sh->widget==widget)
                    addOTW=true;
            }
        }
        if(addOTW)
        {
            foreach(int i,*groups)
            {
                if(defaultReloadGroups.contains(i))
                {
                    defaultReloadGroups.value(i)->append(oTw);
                }
                else
                {
                    defaultReloadGroups.insert(i,new QList<objectToWidget*>());
                    defaultReloadGroups.value(i)->append(oTw);
                }
            }
        }
    }
}
/**
 * Adds a button to a default group
 * @param button pointer to the default button
 * @param buttongroup number of the group
 */
void ConfigTaskWidget::addDefaultButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    connect(button,SIGNAL(clicked()),this,SLOT(defaultButtonClicked()));
}
/**
 * Adds a button to a reload group
 * @param button pointer to the reload button
 * @param buttongroup number of the group
 */
void ConfigTaskWidget::addReloadButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    reloadButtonList.append(button);
    connect(button,SIGNAL(clicked()),this,SLOT(reloadButtonClicked()));
}
/**
 * Adds a button to reboot board
 * @param button pointer to the reload button
 * @param buttongroup number of the group
 */
void ConfigTaskWidget::addRebootButton(QPushButton *button)
{
    rebootButtonList.append(button);
    connect(button, SIGNAL(clicked()), this, SLOT(rebootButtonClicked()));
}

void ConfigTaskWidget::addConnectionsButton(QPushButton *button)
{
    connectionsButtonList.append(button);
    connect(button, SIGNAL(clicked()), this, SLOT(connectionsButtonClicked()));
}

/**
 * Called when a default button is clicked
 */
void ConfigTaskWidget::defaultButtonClicked()
{
    int group = sender()->property("group").toInt();
    emit defaultRequested(group);
    QList<objectToWidget *> *list = defaultReloadGroups.value(group);
    foreach(objectToWidget *oTw, *list) {
        auto object = oTw->object.toStrongRef().dynamicCast<UAVDataObject>();
        auto field = oTw->field.toStrongRef();
        if (object && field) {
            auto temp = object->dirtyClone();
            if (temp)
                setWidgetFromField(oTw->widget, temp->getField(field->getName()), oTw->index, oTw->scale, oTw->isLimited, oTw->useUnits);
        }
    }
}

// TODO: just call into Uploader instead
void ConfigTaskWidget::rebootButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());

    QPointer<UAVObjectManager> objMgr = getObjectManager();
    if (!objMgr) {
        button->setIcon(QIcon(":/uploader/images/error.svg"));
        return;
    }
    auto iapObj = FirmwareIAPObj::getInstance(objMgr);
    if (!iapObj) {
        Q_ASSERT(false);
        qWarning() << "Cannot get FirmwareIAPObj!";
        button->setIcon(QIcon(":/uploader/images/error.svg"));
        return;
    }

    Core::ConnectionManager *conMngr = Core::ICore::instance()->connectionManager();
    if (!conMngr->isConnected() || !iapObj->getIsPresentOnHardware()) {
        button->setIcon(QIcon(":/uploader/images/error.svg"));
        return;
    }

    setWidgetEnabledByObj(button, false);
    button->setIcon(QIcon(":/uploader/images/system-run.svg"));

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    iapObj->setBoardRevision(0);
    iapObj->setBoardType(0);
    connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(iapObj.data(), static_cast<void (UAVObject::*)(QSharedPointer<UAVObject>, bool)>(&UAVObject::transactionCompleted),
            &loop, &QEventLoop::quit);
    int magicValue = 1122;
    int magicStep = 1111;
    for (int i = 0; i < 3; ++i) {
        //Firmware IAP module specifies that the timing between iap commands must be
        //between 500 and 5000ms
        timeout.start(600);
        loop.exec();
        iapObj->setCommand(magicValue);
        magicValue += magicStep;
        if(magicValue == 3344)
            magicValue = 4455;
        iapObj->updated();
        timeout.start(1000);
        loop.exec();
        if(!timeout.isActive())
        {
            setWidgetEnabledByObj(button, true);
            button->setIcon(QIcon(":/uploader/images/error.svg"));
            return;
        }
        timeout.stop();
    }
    setWidgetEnabledByObj(button, true);
    button->setIcon(QIcon(":/uploader/images/dialog-apply.svg"));
    conMngr->disconnectDevice();
}

/**
 * Called when a reload button is clicked
 */
void ConfigTaskWidget::reloadButtonClicked()
{
    if (timeOut)
        return;
    if (!sender() || !sender()->property("group").isValid()) {
        Q_ASSERT(false);
        qWarning() << "reloadButtonClicked called externally!";
    }

    int group = sender()->property("group").toInt();
    auto list = defaultReloadGroups.value(group, NULL);
    if (!list)
        return;

    auto objMgr = getObjectManager();
    if (!objMgr) {
        Q_ASSERT(false);
        qWarning() << "Could not get UAVObjectManager!";
        return;
    }
    auto objPer = ObjectPersistence::getInstance(objMgr);
    if (!objPer) {
        Q_ASSERT(false);
        qWarning() << "Could not get ObjectPersistence!";
        return;
    }

    timeOut = new QTimer(this);
    QEventLoop *eventLoop = new QEventLoop(this);
    connect(timeOut, SIGNAL(timeout()),eventLoop,SLOT(quit()));
    connect(objPer.data(), &UAVObject::objectUpdated, eventLoop, &QEventLoop::quit);

    QList<temphelper> temp;
    for (objectToWidget *oTw : *list) {
        auto obj = oTw->object.toStrongRef();
        if (obj) {
            auto dobj = obj.dynamicCast<UAVDataObject>();
            if (dobj)
                if(!dobj->getIsPresentOnHardware())
                    continue;
            temphelper value;
            value.objid = obj->getObjID();
            value.objinstid = obj->getInstID();
            if (temp.contains(value))
                continue;
            else
                temp.append(value);
            ObjectPersistence::DataFields data;
            data.Operation = ObjectPersistence::OPERATION_LOAD;
            data.ObjectID = obj->getObjID();
            data.InstanceID = obj->getInstID();
            objPer->setData(data);
            objPer->updated();
            timeOut->start(500);
            eventLoop->exec();
            if (timeOut->isActive()) {
                obj->requestUpdate();
                if (oTw->widget)
                    setWidgetFromField(oTw->widget, oTw->field, oTw->index, oTw->scale, oTw->isLimited, oTw->useUnits);
            }
            timeOut->stop();
        }
    }

    if (eventLoop) {
        eventLoop->deleteLater();
        eventLoop = Q_NULLPTR;
    }
    if (timeOut) {
        timeOut->deleteLater();
        timeOut = Q_NULLPTR;
    }
}

void ConfigTaskWidget::connectionsButtonClicked()
{
    ConnectionDiagram diagram(this);
    diagram.exec();
}

void ConfigTaskWidget::doRefreshHiddenObjects(QSharedPointer<UAVDataObject> dobj)
{
    if (!dobj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object!";
        return;
    }

    for (objectToWidget *ow : shadowsList.values()) {
        auto obj = ow->object.toStrongRef();
        if (obj && ow->widget && obj == dobj) {
            for (QWidget *w : shadowsList.keys(ow))
                setWidgetEnabledByObj(w, dobj->getIsPresentOnHardware());
        }
    }
}

/**
 * Connects widgets "contents changed" signals to a slot
 */
void ConfigTaskWidget::connectWidgetUpdatesToSlot(QWidget * widget,const char* function)
{
    if(!widget)
        return;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        connect(cb,SIGNAL(currentIndexChanged(int)),this,function);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        connect(cb,SIGNAL(curveUpdated()),this,function);
    }
    else if(QTableWidget * cb=qobject_cast<QTableWidget *>(widget))
    {
        connect(cb,SIGNAL(cellChanged(int,int)),this,function);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(double)),this,function);
    }
    else if(QGroupBox * cb=qobject_cast<QGroupBox *>(widget))
    {
        connect(cb,SIGNAL(toggled(bool)),this,function);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        connect(cb,SIGNAL(stateChanged(int)),this,function);
    }
    else if(QPushButton * cb=qobject_cast<QPushButton *>(widget))
    {
        connect(cb,SIGNAL(clicked()),this,function);
    }
    else if(qobject_cast<QLabel *>(widget))
    { // Nothing to connect
    }
    else if(qobject_cast<QLineEdit *>(widget))
    { // Nothing to connect
    }
    else
        qDebug() << __FUNCTION__ << "widget to uavobject relation not implemented for widget: " << widget->objectName()  << "of class:" << widget->metaObject()->className();

}
/**
 * Disconnects widgets "contents changed" signals to a slot
 */
void ConfigTaskWidget::disconnectWidgetUpdatesToSlot(QWidget * widget,const char* function)
{
    if(!widget)
        return;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        disconnect(cb,SIGNAL(currentIndexChanged(int)),this,function);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        disconnect(cb,SIGNAL(curveUpdated()),this,function);
    }
    else if(QTableWidget * cb=qobject_cast<QTableWidget *>(widget))
    {
        disconnect(cb,SIGNAL(cellChanged(int,int)),this,function);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(double)),this,function);
    }
    else if(QGroupBox * cb=qobject_cast<QGroupBox *>(widget))
    {
        disconnect(cb,SIGNAL(toggled(bool)),this,function);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        disconnect(cb,SIGNAL(stateChanged(int)),this,function);
    }
    else if(QPushButton * cb=qobject_cast<QPushButton *>(widget))
    {
        disconnect(cb,SIGNAL(clicked()),this,function);
    }
    else if(qobject_cast<QLabel *>(widget))
    { // Nothing to disconnect
    }
    else if(qobject_cast<QLineEdit *>(widget))
    { // Nothing to disconnect
    }
    else
        qDebug() << __FUNCTION__ << "widget to uavobject relation not implemented for widget: " << widget->objectName()  << "of class:" << widget->metaObject()->className();

}

/**
 * Sets a widget value from an UAVObject field
 * @param widget pointer for the widget to set
 * @param field pointer to the UAVObject field to use
 * @param index index of the element to use
 * @param scale scale to be used on the assignement
 * @return returns true if the assignement was successfull
 */
bool ConfigTaskWidget::setFieldFromWidget(QWidget *widget, QSharedPointer<UAVObjectField> field, int index, double scale, bool usesUnits)
{
    QPointer<QWidget> wid = widget;
    if (!wid || !field || qobject_cast<QLabel *>(widget)) // Labels are readonly
        return false;

    QVariant ret = getVariantFromWidget(wid, scale, usesUnits);
    if (ret.isValid()) {
        field->setValue(ret, index);
        return true;
    } else {
        qWarning() << __FUNCTION__ << "widget to uavobject relation not implemented for widget: " << wid->objectName()  << "of class:" << wid->metaObject()->className();
        return false;
    }
}

/**
 * Gets a variant from a widget
 * @param widget pointer to the widget from where to get the value
 * @param scale scale to be used on the assignement
 * @return returns the value of the widget times the scale
 */
QVariant ConfigTaskWidget::getVariantFromWidget(QWidget *widget, double scale, bool usesUnits)
{
    if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget)) {
        return comboBox->currentData();
    } else if (QDoubleSpinBox *dblSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
        return (double)(dblSpinBox->value() * scale);
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget)) {
        return (double)(spinBox->value() * scale);
    } else if (QSlider *slider = qobject_cast<QSlider *>(widget)) {
        return (double)(slider->value() * scale);
    } else if (QGroupBox *groupBox = qobject_cast<QGroupBox *>(widget)) {
        return getOptionFromChecked(widget, groupBox->isChecked());
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget)) {
        return getOptionFromChecked(widget, checkBox->isChecked());
    } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget)) {
        QString text = lineEdit->displayText();
        if (text.startsWith("0b") || text.startsWith("0o") || text.startsWith("0x"))
            text.remove(0, 2);
        if (usesUnits) {
            QStringList bits = text.split(' ');
            if (bits.length())
                bits.removeLast();
            return bits.join(' ');
        } else {
            return text;
        }
    } else {
        return QVariant();
    }
}

/**
 * Sets a widget from a variant
 * @param widget pointer for the widget to set
 * @param scale scale to be used on the assignement
 * @param value value to be used on the assignement
 * @return returns true if the assignement was successfull
 */
bool ConfigTaskWidget::setWidgetFromVariant(QWidget *widget, QVariant value, double scale, int radix, QString units)
{
    units = units.trimmed();
    if (!units.isEmpty() && !qFuzzyCompare(1 + 1.0, 1 + scale) && scale != 0)
        units = applyScaleToUnits(units, scale);
    if(!units.isEmpty())
        units.prepend(' ');

    QString prefix;
    switch (radix) {
    case UAVObjectField::BIN:
        prefix = "0b";
        break;
    case UAVObjectField::OCT:
        prefix = "0o";
        break;
    case UAVObjectField::HEX:
        prefix = "0x";
        break;
    default:
        break;
    }

    if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget)) {
        comboBox->setCurrentIndex(comboBox->findData(value.toString()));
        return true;
    } else if (QLabel *label = qobject_cast<QLabel *>(widget)) {
        QString text;
        if (qFuzzyCompare(scale, 1)) {
            switch (static_cast<QMetaType::Type>(value.type())) {
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Char:
            case QMetaType::SChar:
                text = prefix + QString::number(value.toInt(), radix);
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::UShort:
            case QMetaType::UChar:
                text = prefix + QString::number(value.toUInt(), radix);
                break;
            default:
                text = value.toString();
            }
        } else {
            text = QString::number((value.toDouble() / scale));
        }
        label->setText(text + units);
        return true;
    } else if (QDoubleSpinBox *dblSpinBox = qobject_cast<QDoubleSpinBox *>(widget)) {
        dblSpinBox->setValue(value.toDouble() / scale);
        if (!units.isEmpty())
            dblSpinBox->setSuffix(units);
        return true;
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget)) {
        spinBox->setValue(qRound(value.toDouble() / scale));
        spinBox->setDisplayIntegerBase(radix);
        spinBox->setPrefix(prefix);
        if (!units.isEmpty())
            spinBox->setSuffix(units);
        return true;
    } else if (QSlider *slider = qobject_cast<QSlider *>(widget)) {
        slider->setValue(qRound(value.toDouble() / scale));
        return true;
    } else if (QGroupBox *groupBox = qobject_cast<QGroupBox *>(widget)) {
        groupBox->setChecked(getCheckedFromOption(widget, value.toString()));
        return true;
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget)) {
        checkBox->setChecked(getCheckedFromOption(widget, value.toString()));
        return true;
    } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget)) {
        QString text;
        if (qFuzzyCompare(scale, 1)) {
            switch (static_cast<QMetaType::Type>(value.type())) {
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Char:
            case QMetaType::SChar:
                text = prefix + QString::number(value.toInt(), radix);
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::UShort:
            case QMetaType::UChar:
                text = prefix + QString::number(value.toUInt(), radix);
                break;
            default:
                text = value.toString();
            }
        } else {
            text = QString::number((value.toDouble() / scale));
        }
        lineEdit->setText(text + units);
        return true;
    }

    return false;
}

bool ConfigTaskWidget::setWidgetFromField(QWidget *widget, QSharedPointer<UAVObjectField> field, int index, double scale, bool hasLimits, bool useUnits)
{
    QPointer<QWidget> wid(widget);
    if (!wid || !field)
        return false;

    // use UAVO field description as tooltip if the widget doesn't already have one
    if (!wid->toolTip().length()) {
        QString desc = field->getDescription().trimmed().toHtmlEscaped();
        if (desc.length()) {
            // insert html tags to make this rich text so Qt will take care of wrapping
            desc.prepend("<span style='font-style: normal'>");
            desc.remove("@Ref", Qt::CaseInsensitive);
            desc.append("</span>");
        }
        wid->setToolTip(desc);
    }

    if (QComboBox *cb = qobject_cast<QComboBox *>(wid)) {
        if (cb->count() == 0)
            loadWidgetLimits(cb, field, index, hasLimits, useUnits, scale);
    }

    QVariant var = field->getValue(index);
    checkWidgetsLimits(widget, field, index, hasLimits, useUnits, var, scale);
    const QString units = useUnits ? field->getUnits() : "";
    bool ret = setWidgetFromVariant(widget, var, scale, field->getRadix(), units);

    if (!ret)
        qDebug() << __FUNCTION__ << "widget to uavobject relation not implemented for widget: " << widget->objectName()  << "of class:" << widget->metaObject()->className();
    return ret;
}

void ConfigTaskWidget::checkWidgetsLimits(QWidget *widget, QSharedPointer<UAVObjectField> field, int index, bool hasLimits, bool useUnits, QVariant value, double scale)
{
    if (!hasLimits || !field)
        return;

    if (!field->isWithinLimits(value, index, currentBoard)) {
        if (!widget->property("styleBackup").isValid())
            widget->setProperty("styleBackup",widget->styleSheet());
        widget->setStyleSheet(outOfLimitsStyle);
        widget->setProperty("wasOverLimits", true);
        if (!widget->property("toolTipBackup").isValid()) {
            QString tip = widget->toolTip();
            if (tip.length() && !tip.startsWith("<"))
                tip = tip.prepend("<p>").append("</p>");
            widget->setProperty("toolTipBackup", tip);
        }
        widget->setToolTip(widget->property("toolTipBackup").toString() +
                           tr("<p><strong>Warning:</strong> The value of this field exceeds the recommended limits! Please double-check before flying.</p>"));
        if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
        {
            if(cb->findData(value.toString())==-1)
                cb->addItem(value.toString(), value);
        }
        else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
        {
            if((double)(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((double)(value.toDouble()/scale));
            }
            else if((double)(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((double)(value.toDouble()/scale));
            }

        }
        else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
        {
            if((int)qRound(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((int)qRound(value.toDouble()/scale));
            }
            else if((int)qRound(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((int)qRound(value.toDouble()/scale));
            }
        }
        else if(QSlider * cb=qobject_cast<QSlider *>(widget))
        {
            if((int)qRound(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((int)qRound(value.toDouble()/scale));
            }
            else if((int)qRound(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((int)qRound(value.toDouble()/scale));
            }
        }

    }
    else if(widget->property("wasOverLimits").isValid())
    {
        if(widget->property("wasOverLimits").toBool())
        {
            widget->setProperty("wasOverLimits",(bool)false);
            if(widget->property("styleBackup").isValid())
            {
                QString style=widget->property("styleBackup").toString();
                widget->setStyleSheet(style);
            }

            if(widget->property("toolTipBackup").isValid())
                widget->setToolTip(widget->property("toolTipBackup").toString());
            else
                widget->setToolTip("");

            loadWidgetLimits(widget, field, index, hasLimits, useUnits, scale);
        }
    }
}

void ConfigTaskWidget::loadWidgetLimits(QWidget * widget, QSharedPointer<UAVObjectField> field, int index, bool hasLimits, bool useUnits, double scale)
{   
    if (!widget || !field)
        return;

    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        cb->clear();
        QStringList option=field->getOptions();
        foreach(QString str,option)
        {
            if(!hasLimits || field->isWithinLimits(str,index,currentBoard)) {
                if (useUnits)
                    cb->addItem(str + " " + field->getUnits(), str);
                else
                    cb->addItem(str, str);
            }
        }
    }
    if(!hasLimits)
        return;
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        if(field->getMaxLimit(index).isValid())
        {
            cb->setMaximum((double)(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((double)(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        if(field->getMaxLimit(index,currentBoard).isValid())
        {
            cb->setMaximum((int)qRound(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((int)qRound(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        if(field->getMaxLimit(index,currentBoard).isValid())
        {
            cb->setMaximum((int)qRound(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((int)(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
}

void ConfigTaskWidget::disableMouseWheelEvents()
{
    //Disable mouse wheel events
    foreach( QSpinBox * sp, findChildren<QSpinBox*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QDoubleSpinBox * sp, findChildren<QDoubleSpinBox*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QSlider * sp, findChildren<QSlider*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QComboBox * sp, findChildren<QComboBox*>() ) {
        sp->installEventFilter( this );
    }
}

bool ConfigTaskWidget::eventFilter( QObject * obj, QEvent * evt ) {
    //Filter all wheel events, and ignore them
    if ( evt->type() == QEvent::Wheel &&
         (qobject_cast<QAbstractSpinBox*>( obj ) ||
          qobject_cast<QComboBox*>( obj ) ||
          qobject_cast<QAbstractSlider*>( obj ) ))
    {
        evt->ignore();
        return true;
    }
    return QWidget::eventFilter( obj, evt );
}

/**
 * @brief Determine which enum option based on checkbox
 * @param widget Target widget
 * @param checked Checked status of widget
 * @return Enum option as string
 */
QString ConfigTaskWidget::getOptionFromChecked(QWidget *widget, bool checked)
{
    if (checked)
        return widget->property("checkedOption").isValid() ? widget->property("checkedOption").toString() : "TRUE";
    else
        return widget->property("unCheckedOption").isValid() ? widget->property("unCheckedOption").toString() : "FALSE";
}

/**
 * @brief Determine whether checkbox should be checked
 * @param widget Target widget
 * @param option Enum option as string
 * @return
 */
bool ConfigTaskWidget::getCheckedFromOption(QWidget *widget, QString option)
{
    if (widget->property("checkedOption").isValid())
        return option == widget->property("checkedOption").toString();
    if (widget->property("unCheckedOption").isValid())
        return option != widget->property("unCheckedOption").toString();
    return option == "TRUE";
}

enum UnitPrefixes {
    PREFIX_NANO,
    PREFIX_MICRO,
    PREFIX_MILLI,
    PREFIX_NONE,
    PREFIX_KILO,
    PREFIX_MEGA,
    PREFIX_GIGA,
};

QString ConfigTaskWidget::applyScaleToUnits(QString units, double scale)
{
    // if no scaling is applied (scale 0 or 1), return units unchanged
    if (qFuzzyCompare(1, 1 + scale) || qFuzzyCompare(1.0, scale))
        return units;

    int len = units.length();
    // only consider basic units with length 1 (no prefix), or 2 atm
    if (!len || len > 2)
        return QString();

    int prefix = PREFIX_NONE;
    if (len > 1) {
        QChar p = units.at(0);
        if (p == 'n') // nano
            prefix = PREFIX_NANO;
        else if (p == 'u' || p == QString::fromLatin1("\xb5s")) // micro
            prefix = PREFIX_MICRO;
        else if (p == 'm') // milli
            prefix = PREFIX_MILLI;
        else if (p == 'k') // kilo
            prefix = PREFIX_KILO;
        else if (p == 'M') // Mega
            prefix = PREFIX_MEGA;
        else if (p == 'G') // Giga
            prefix = PREFIX_GIGA;
        else
            return QString();
        units = units.at(1);
    }

    if (qFuzzyCompare(1.0e-9, scale))
        prefix -= 3;
    else if (qFuzzyCompare(1.0e-6, scale))
        prefix -= 2;
    else if (qFuzzyCompare(1.0e-3, scale))
        prefix -= 1;
    else if (qFuzzyCompare(1.0e3, scale))
        prefix += 1;
    else if (qFuzzyCompare(1.0e6, scale))
        prefix += 2;
    else if (qFuzzyCompare(1.0e9, scale))
        prefix += 3;
    else
        return QString();

    switch (prefix) {
    case PREFIX_NANO:
        return "n" + units;
    case PREFIX_MICRO:
        return QString::fromLatin1("\xb5") + units;
    case PREFIX_MILLI:
        return "m" + units;
    case PREFIX_NONE:
        return units;
    case PREFIX_KILO:
        return "k" + units;
    case PREFIX_MEGA:
        return "M" + units;
    case PREFIX_GIGA:
        return "G" + units;
    default:
        return QString();
    }
}

bool ConfigTaskWidget::resetWidgetToDefault(QWidget *widget)
{
    for (objectToWidget *ow : objOfInterest) {
        auto field = ow->field.toStrongRef();
        if (ow->widget == widget && field)
            return setWidgetFromVariant(widget, field->getDefaultValue(ow->index), ow->scale, field->getRadix(), ow->useUnits ? field->getUnits() : "");
    }
    return false;
}

void ConfigTaskWidget::setWidgetProperty(QWidget *widget, const char *prop, const QVariant &value)
{
    widget->setProperty(prop, value);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

void ConfigTaskWidget::setWidgetEnabled(QWidget *widget, bool enabled)
{
    bool objDisabled = false;
    objDisabled = widget->property("objDisabled").toBool();
    widget->setProperty("userDisabled", !enabled);
    widget->setEnabled(enabled && !objDisabled);
}

void ConfigTaskWidget::setWidgetEnabledByObj(QWidget *widget, bool enabled)
{
    bool userDisabled = false;
    userDisabled = widget->property("userDisabled").toBool();
    widget->setProperty("objDisabled", !enabled);
    widget->setEnabled(enabled && !userDisabled);
}

/**
  @}
  @}
  */
