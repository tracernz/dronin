/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A place holder gadget plugin
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
 * with this program; if not, see <http://www.gnu.org/licenses/>
 */

#include "gcscontrolgadgetoptionspage.h"
#include "gcscontrolgadgetconfiguration.h"
#include "ui_gcscontrolgadgetoptionspage.h"
#include "gcscontrol.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include <QGamepad>

GCSControlGadgetOptionsPage::GCSControlGadgetOptionsPage(GCSControlGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config),
        m_gamepad(Q_NULLPTR)
{
    options_page = NULL;

#if defined(USE_SDL)
    sdlGamepad = dynamic_cast<GCSControl*>(parent)->sdlGamepad;
#endif
}

GCSControlGadgetOptionsPage::~GCSControlGadgetOptionsPage()
{
    disconnect(this);
    externalDeviceChanged(0);
}

void GCSControlGadgetOptionsPage::gamepads(quint8 count)
{
    Q_UNUSED(count);
}

#if defined(USE_SDL)
void GCSControlGadgetOptionsPage::buttonState(ButtonNumber number, bool pressed)
{
    if (options_page) {
        QList<QCheckBox*> rbList;
        rbList << options_page->buttonInput0 <<
                  options_page->buttonInput1 << options_page->buttonInput2 <<
                  options_page->buttonInput3 << options_page->buttonInput4 <<
                  options_page->buttonInput5 << options_page->buttonInput6 <<
                  options_page->buttonInput7;

        if (number<8) // We only support 8 buttons
        {
            rbList.at(number)->setChecked(pressed);
        }
    }

}

void GCSControlGadgetOptionsPage::axesValues(QListInt16 values)
{
    if (options_page) {
        QList<QProgressBar*> pbList;
        pbList << options_page->joyCh0 <<
                  options_page->joyCh1 << options_page->joyCh2 <<
                  options_page->joyCh3 << options_page->joyCh4 <<
                  options_page->joyCh5 << options_page->joyCh6 <<
                  options_page->joyCh7;
        int i=0;
        foreach (qint16 value, values) {
            if (i>7) break; // We only support 7 channels
            if (chRevList.at(i)->isChecked()==1)value = 65535 - value;
             if (pbList.at(i)->minimum() > value)
                 pbList.at(i)->setMinimum(value);
             if (pbList.at(i)->maximum() < value)
                 pbList.at(i)->setMaximum(value);
            pbList.at(i++)->setValue(value);
        }
    }
}
#endif

//creates options page widget (uses the UI file)
QWidget *GCSControlGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    int i;
    options_page = new Ui::GCSControlGadgetOptionsPage();
    QWidget *optionsPageWidget = new QWidget;
    options_page->setupUi(optionsPageWidget);

    options_page->gcsReceiverCB->setChecked(m_config->getGcsReceiverMode());

    //QList<QComboBox*> chList;
    chList.clear();
    chList << options_page->channel0 << options_page->channel1 <<
              options_page->channel2 << options_page->channel3 <<
              options_page->channel4 << options_page->channel5 <<
              options_page->channel6 << options_page->channel7;
    QStringList chOptions;
    chOptions << "None" << "Roll" << "Pitch" << "Yaw" << "Throttle";
    foreach (QComboBox* qb, chList) {
        qb->addItems(chOptions);
    }
    //QList<QCheckBox*> chRevList;
    chRevList.clear();
    chRevList << options_page->revCheckBox_1 << options_page->revCheckBox_2 <<
                 options_page->revCheckBox_3 << options_page->revCheckBox_4 <<
                 options_page->revCheckBox_5 << options_page->revCheckBox_6 <<
                 options_page->revCheckBox_7 << options_page->revCheckBox_8;

    //QList<QComboBox*> buttonFunctionList;
    buttonFunctionList.clear();
    buttonFunctionList << options_page->buttonFunction0 << options_page->buttonFunction1 <<
              options_page->buttonFunction2 << options_page->buttonFunction3 <<
              options_page->buttonFunction4 << options_page->buttonFunction5 <<
              options_page->buttonFunction6 << options_page->buttonFunction7;
    QStringList buttonOptions;
    buttonOptions <<"-" << "Roll" << "Pitch" << "Yaw" << "Throttle" << "Armed" << "GCS Control"; //added UDP control to action list
    foreach (QComboBox* qb, buttonFunctionList) {
        qb->addItems(buttonOptions);
    }
    //QList<QComboBox*> buttonActionList;
    buttonActionList.clear();
    buttonActionList << options_page->buttonAction0 << options_page->buttonAction1 <<
              options_page->buttonAction2 << options_page->buttonAction3 <<
              options_page->buttonAction4 << options_page->buttonAction5 <<
              options_page->buttonAction6 << options_page->buttonAction7;
    QStringList buttonActionOptions;
    buttonActionOptions << "Does nothing" << "Increases" << "Decreases" << "Toggles";
    foreach (QComboBox* qb, buttonActionList) {
        qb->addItems(buttonActionOptions);
    }
    //QList<QDoubleSpinBox*> buttonValueList;
    buttonValueList.clear();
    buttonValueList << options_page->buttonAmount0 << options_page->buttonAmount1 <<
              options_page->buttonAmount2 << options_page->buttonAmount3 <<
              options_page->buttonAmount4 << options_page->buttonAmount5 <<
              options_page->buttonAmount6 << options_page->buttonAmount7;
    //QList<QLabel*> buttonLabelList;
    buttonLabelList.clear();
    buttonLabelList << options_page->buttonLabel0 << options_page->buttonLabel1 <<
              options_page->buttonLabel2 << options_page->buttonLabel3 <<
              options_page->buttonLabel4 << options_page->buttonLabel5 <<
              options_page->buttonLabel6 << options_page->buttonLabel7;

    for (i=0;i<8;i++)
    {
        buttonActionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).ActionID);
        buttonFunctionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).FunctionID);
        buttonValueList.at(i)->setValue(m_config->getbuttonSettings(i).Amount);

        connect(buttonFunctionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        //connect(buttonActionList.at(i),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonActions[i]()));
        updateButtonAction(i);
        buttonFunctionList.at(i)->setCurrentIndex(m_config->getbuttonSettings(i).FunctionID);
    }
    connect(buttonActionList.at(0),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_0()));
    connect(buttonActionList.at(1),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_1()));
    connect(buttonActionList.at(2),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_2()));
    connect(buttonActionList.at(3),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_3()));
    connect(buttonActionList.at(4),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_4()));
    connect(buttonActionList.at(5),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_5()));
    connect(buttonActionList.at(6),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_6()));
    connect(buttonActionList.at(7),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonAction_7()));

    //updateButtonFunction();

    options_page->udp_host->setText(m_config->getUDPControlHost().toString());
    options_page->udp_port->setText(QString::number(m_config->getUDPControlPort()));


    // Controls mode are from 1 to 4.
    if (m_config->getControlsMode()>0 && m_config->getControlsMode() < 5)
        options_page->controlsMode->setCurrentIndex(m_config->getControlsMode()-1);
    else
        qDebug() << "GCSControl: Invalid control modes setting! Did you edit by hand?";

    QList<int> ql = m_config->getChannelsMapping();
    for (int i=0; i<4; i++) {
        if (ql.at(i) > -1)
            chList.at(ql.at(i))->setCurrentIndex(i+1);
    }
    QList<bool> qlChRev = m_config->getChannelsReverse();
    for (i=0; i<8; i++)
    {
        chRevList.at(i)->setChecked(qlChRev.at(i));;
    }

#if defined(USE_SDL)
    connect(sdlGamepad,SIGNAL(axesValues(QListInt16)),this,SLOT(axesValues(QListInt16)));
    connect(sdlGamepad,SIGNAL(buttonState(ButtonNumber,bool)),this,SLOT(buttonState(ButtonNumber,bool)));
    connect(sdlGamepad,SIGNAL(gamepads(quint8)),this,SLOT(gamepads(quint8)));
#endif

    connect(options_page->cbExternalDevice, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GCSControlGadgetOptionsPage::externalDeviceChanged);

    auto padMgr = QGamepadManager::instance();
    if (padMgr) {
        connect(padMgr, &QGamepadManager::connectedGamepadsChanged, this,
                &GCSControlGadgetOptionsPage::connectedGamepadsChanged);
        connectedGamepadsChanged();
    }
    connect(padMgr, &QGamepadManager::gamepadAxisEvent, this, [](int devId, QGamepadManager::GamepadAxes axis, double value) {
        qDebug() << "axis event" << devId << axis << value;
    });

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void GCSControlGadgetOptionsPage::apply()
{
   m_config->setControlsMode(options_page->controlsMode->currentIndex()+1);

   int roll=-1 , pitch=-1, yaw=-1, throttle=-1;
   for (int i=0; i<chList.length(); i++) {
      switch (chList.at(i)->currentIndex()) {
      case 1:
          roll = i;
          break;
      case 2:
          pitch =i;
         break;
      case 3:
          yaw = i;
          break;
      case 4:
          throttle = i;
          break;
      }
   }
   m_config->setRPYTchannels(roll,pitch,yaw,throttle);

   m_config->setUDPControlSettings(options_page->udp_port->text().toInt(),options_page->udp_host->text());

   m_config->setGcsReceiverMode(options_page->gcsReceiverCB->checkState());

   for (unsigned int j = 0; j < 8; j++)
   {
       m_config->setbuttonSettingsAction(j,buttonActionList.at(j)->currentIndex());
       m_config->setbuttonSettingsFunction(j,buttonFunctionList.at(j)->currentIndex());
       m_config->setbuttonSettingsAmount(j,buttonValueList.at(j)->value());
       m_config->setChannelReverse(j,chRevList.at(j)->isChecked());
   }


}

void GCSControlGadgetOptionsPage::finish()
{
#if defined(USE_SDL)
    disconnect(sdlGamepad,0,this,0);
#endif
    delete options_page;
    options_page = NULL;
}


void GCSControlGadgetOptionsPage::updateButtonFunction()
{
    for (unsigned int i=0;i<8;i++)
    {
        if (buttonActionList.at(i)->currentText().compare("Does nothing")==0)
        {
            buttonFunctionList.at(i)->setVisible(0);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
        }
        else
        if (buttonActionList.at(i)->currentText().compare("Toggles")==0)
        {
            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(0);
            buttonValueList.at(i)->setVisible(0);
       }
        else
        {
            buttonFunctionList.at(i)->setVisible(1);
            buttonLabelList.at(i)->setVisible(1);
            buttonValueList.at(i)->setVisible(1);
        }
    }


}

void GCSControlGadgetOptionsPage::updateButtonAction(int controlID)
{
    QStringList buttonOptions;

    {
        if (buttonActionList.at(controlID)->currentText().compare("Does nothing")==0)
        {
            buttonFunctionList.at(controlID)->setVisible(0);
            buttonLabelList.at(controlID)->setVisible(0);
            buttonValueList.at(controlID)->setVisible(0);
        }
        else
        if (buttonActionList.at(controlID)->currentText().compare("Toggles")==0)
        {
            disconnect(buttonFunctionList.at(controlID),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
            buttonOptions <<"-" << "Armed" << "GCS Control" << "UDP Control";
            buttonFunctionList.at(controlID)->clear();
            buttonFunctionList.at(controlID)->insertItems(-1,buttonOptions);

            buttonFunctionList.at(controlID)->setVisible(1);
            buttonLabelList.at(controlID)->setVisible(0);
            buttonValueList.at(controlID)->setVisible(0);
            connect(buttonFunctionList.at(controlID),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        }
        else
        {
            disconnect(buttonFunctionList.at(controlID),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
            buttonOptions <<"-" << "Roll" << "Pitch" << "Yaw" << "Throttle" ;
            buttonFunctionList.at(controlID)->clear();
            buttonFunctionList.at(controlID)->addItems(buttonOptions);

            buttonFunctionList.at(controlID)->setVisible(1);
            buttonLabelList.at(controlID)->setVisible(1);
            buttonValueList.at(controlID)->setVisible(1);
            connect(buttonFunctionList.at(controlID),SIGNAL(currentIndexChanged(int)),this,SLOT(updateButtonFunction()));
        }
    }


}

void GCSControlGadgetOptionsPage::connectedGamepadsChanged()
{
    auto padMgr = QGamepadManager::instance();
    if (!padMgr || !options_page)
        return;

    auto pads = padMgr->connectedGamepads();

    for (int i = 0; i < options_page->cbExternalDevice->count(); ) {
        auto devId = options_page->cbExternalDevice->itemData(i).value<ExternalDeviceId>();
        if (devId.first == GAMEPAD && !pads.contains(devId.second)) {
            options_page->cbExternalDevice->removeItem(i);
        } else {
            if (pads.contains(devId.second))
                pads.removeAt(pads.indexOf(devId.second));
            i++;
        }
    }

    for (auto pad : pads)
        options_page->cbExternalDevice->addItem(QString("Joystick [%0]").arg(pad), QVariant::fromValue(ExternalDeviceId(GAMEPAD, pad)));
}

void GCSControlGadgetOptionsPage::externalDeviceChanged(int index)
{
    if (m_gamepad) {
        m_gamepad->disconnect();
        m_gamepad->deleteLater();
        m_gamepad = Q_NULLPTR;
    }

    if (index <= 0)
        return;

    auto devId = options_page->cbExternalDevice->itemData(index).value<ExternalDeviceId>();
    switch (devId.first) {
    case GAMEPAD:
        m_gamepad = new QGamepad(devId.second, this);
        if (m_gamepad) {
            connect(m_gamepad, &QGamepad::axisLeftXChanged, this, [this](double value) {
                this->axisChanged(0, 0.5 + value / 2.0);
            });
            axisChanged(0, 0.5 + m_gamepad->axisLeftX() / 2.0);
            connect(m_gamepad, &QGamepad::axisLeftYChanged, this, [this](double value) {
                this->axisChanged(1, 0.5 + value / 2.0);
            });
            axisChanged(1, 0.5 + m_gamepad->axisLeftY() / 2.0);
            connect(m_gamepad, &QGamepad::axisRightXChanged, this, [this](double value) {
                this->axisChanged(2, 0.5 + value / 2.0);
            });
            axisChanged(2, 0.5 + m_gamepad->axisRightX() / 2.0);
            connect(m_gamepad, &QGamepad::axisRightYChanged, this, [this](double value) {
                this->axisChanged(3, 0.5 + value / 2.0);
            });
            axisChanged(3, 0.5 + m_gamepad->axisRightY() / 2.0);
            connect(m_gamepad, &QGamepad::buttonL2Changed, this, [this](double value) {
                this->axisChanged(4, value);
            });
            axisChanged(4, m_gamepad->buttonL2());
            connect(m_gamepad, &QGamepad::buttonR2Changed, this, [this](double value) {
                this->axisChanged(5, value);
            });
            axisChanged(5, m_gamepad->buttonR2());

            connect(m_gamepad, &QGamepad::buttonUpChanged, this, [this](bool value) {
                this->buttonChanged(0, value);
            });
            buttonChanged(0, m_gamepad->buttonUp());
            connect(m_gamepad, &QGamepad::buttonLeftChanged, this, [this](bool value) {
                this->buttonChanged(1, value);
            });
            buttonChanged(1, m_gamepad->buttonLeft());
            connect(m_gamepad, &QGamepad::buttonDownChanged, this, [this](bool value) {
                this->buttonChanged(2, value);
            });
            buttonChanged(2, m_gamepad->buttonDown());
            connect(m_gamepad, &QGamepad::buttonRightChanged, this, [this](bool value) {
                this->buttonChanged(3, value);
            });
            buttonChanged(0, m_gamepad->buttonRight());

        }
        break;
    default:
        break;
    }
}

void GCSControlGadgetOptionsPage::axisChanged(int axis, double value)
{
    if (!options_page)
        return;
    auto widget = options_page->widget->findChild<QProgressBar *>(QString("joyCh%0").arg(axis));
    if (widget)
        widget->setValue(static_cast<int>(0.5 + widget->minimum() + value * (widget->maximum() - widget->minimum())));
}

void GCSControlGadgetOptionsPage::buttonChanged(int button, bool value)
{
    if (!options_page)
        return;
    auto widget = options_page->widget->findChild<QCheckBox *>(QString("buttonInput%0").arg(button));
    if (widget)
        widget->setChecked(value);
}
