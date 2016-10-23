/**
 ******************************************************************************
 *
 * @file       configoutputwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     dRonin, http://dronin.org Copyright (C) 2015-2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo output configuration panel for the config gadget
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

#include "configoutputwidget.h"
#include "outputchannelform.h"
#include "outputbankform.h"
#include "configvehicletypewidget.h"

#include "uavtalk/telemetrymanager.h"

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>
#include "mixersettings.h"
#include "actuatorcommand.h"
#include "actuatorsettings.h"
#include "systemalarms.h"
#include "systemsettings.h"
#include "uavsettingsimportexport/uavsettingsimportexportmanager.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>
#include <coreplugin/modemanager.h>

ConfigOutputWidget::ConfigOutputWidget(QWidget *parent) :
    ConfigTaskWidget(parent),
    numBanks(ActuatorSettings::TIMERUPDATEFREQ_NUMELEM)
{
    m_config = new Ui_OutputWidget();
    m_config->setupUi(this);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    /* There's lots of situations where it's unsafe to run tests.
     * Import/export:
     */
    UAVSettingsImportExportManager *importexportplugin = pm->getObject<UAVSettingsImportExportManager>();
    connect(importexportplugin, SIGNAL(importAboutToBegin()), this, SLOT(stopTests()));

    /* Board connection/disconnection: */
    connect(this, SIGNAL(telemetryConnected()), this, SLOT(stopTests()));
    connect(this, SIGNAL(telemetryDisconnected()), this, SLOT(stopTests()));

    /* When we go into wizards, etc.. time to stop this too. */
    Core::ModeManager *modeMngr = Core::ModeManager::instance();
    connect(modeMngr, SIGNAL(currentModeAboutToChange(Core::IMode *)), this,
		SLOT(stopTests()));
    connect(modeMngr, SIGNAL(currentModeChanged(Core::IMode *)), this,
		SLOT(stopTests()));

    // NOTE: we have channel indices from 0 to 9, but the UI convention is Channel 1 to 10.
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
        OutputChannelForm *outputForm = new OutputChannelForm(i, this);
        outputForm->setObjectName(QString("outputChannel%0").arg(i+1));
        m_config->channelLayout->addWidget(outputForm);

        connect(m_config->channelOutTest, SIGNAL(toggled(bool)), outputForm, SLOT(enableChannelTest(bool)));
        connect(outputForm, SIGNAL(channelChanged(int,int)), this, SLOT(sendChannelTest(int,int)));
    }

    for (int i = 0; i < numBanks; i++) {
        OutputBankForm *bankForm = new OutputBankForm(this, i);
        bankForm->setObjectName(QString("outputBank%0").arg(i));
        m_config->channelLayout->addWidget(bankForm);
        outputBanks << bankForm;
        connect(this, SIGNAL(channelBanksUpdated()), bankForm, SLOT(channelsUpdated()));
    }
    // set the banks the first time to setup UI
    emit(channelBanksUpdated());
    // and update it when we have a new board (may have different bank layout)
    connect(this, SIGNAL(telemetryConnected()), this, SLOT(reloadBanks()));

    connect(m_config->channelOutTest, SIGNAL(toggled(bool)), this, SLOT(runChannelTests(bool)));
    connect(m_config->calibrateESC, SIGNAL(clicked()), this, SLOT(startESCCalibration()));

    // Configure the task widget
    autoLoadWidgets();
    populateWidgets();
    refreshWidgetsValues();

    // TODO: is this for HiTL? but why only in constructor then?
    UAVObject *obj = getObject<UAVObject>("ActuatorCommand");
    if(!obj || UAVObject::GetGcsTelemetryUpdateMode(obj->getMetadata()) == UAVObject::UPDATEMODE_ONCHANGE)
        this->setEnabled(false);
    connect(getObject<UAVObject>("SystemSettings"), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(assignOutputChannels(UAVObject*)));

    disableMouseWheelEvents();
}

void ConfigOutputWidget::enableControls(bool enable)
{
    // ConfigTaskWidget takes care of all the widgets with UAVO relations
    ConfigTaskWidget::enableControls(enable);

    // and we take care of the rest
    if(!enable)
        m_config->channelOutTest->setChecked(false);
    m_config->channelOutTest->setEnabled(enable);
    m_config->calibrateESC->setEnabled(enable);
    // including banks and channels
    for (OutputBankForm *bank : outputBanks)
        bank->enableControls(enable);
    for (OutputChannelForm *chan : findChildren<OutputChannelForm *>())
        chan->enableControls(enable);
}

ConfigOutputWidget::~ConfigOutputWidget()
{
   // Do nothing
}

// ************************************

/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigOutputWidget::runChannelTests(bool state)
{
    SystemAlarms * systemAlarmsObj = SystemAlarms::getInstance(getObjectManager());
    if (!systemAlarmsObj)
        return;
    SystemAlarms::DataFields systemAlarms = systemAlarmsObj->getData();

    if(state && ((systemAlarms.Alarm[SystemAlarms::ALARM_ACTUATOR] == SystemAlarms::ALARM_ERROR) || (systemAlarms.Alarm[SystemAlarms::ALARM_ACTUATOR] == SystemAlarms::ALARM_CRITICAL))) {
        QMessageBox mbox;
        mbox.setText(QString(tr("The actuator module is in an error state.  This can also occur because there are no inputs.  Please fix these before testing outputs.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();

        // Unfortunately must cache this since callback will reoccur
        ActuatorCommand *actCommand = ActuatorCommand::getInstance(getObjectManager());
        if (actCommand)
            accInitialData = actCommand->getMetadata();

        m_config->channelOutTest->setChecked(false);
        return;
    }

    // Confirm this is definitely what they want
    if(state) {
        QMessageBox mbox;
        mbox.setText(QString(tr("This option will start your motors by the amount selected on the sliders regardless of transmitter.  It is recommended to remove any blades from motors.  Are you sure you want to do this?")));
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int retval = mbox.exec();
        if(retval != QMessageBox::Yes) {
            state = false;
            m_config->channelOutTest->setChecked(false);
            return;
        }
    }

    ActuatorCommand *obj = ActuatorCommand::getInstance(getObjectManager());
    if (!obj)
        return;
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state) {
        accInitialData = mdata;
        UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(mdata, false);
        UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        mdata.gcsTelemetryUpdatePeriod = 100;
    } else {
        mdata = accInitialData; // Restore metadata
    }
    obj->setMetadata(mdata);
    obj->updated();
}

/**
 * @brief ConfigOutputWidget::assignOutputChannels Sets the output channel form text
 * @param actuatorSettings UAVO input
 */
void ConfigOutputWidget::assignOutputChannels(UAVObject *obj)
{
    Q_UNUSED(obj);

    // Get channel descriptions
    QStringList ChannelDesc = ConfigVehicleTypeWidget::getChannelDescriptions();

    // Find all output forms in the tab, and set the text and min/max values
    for (OutputChannelForm *outputChannelForm : findChildren<OutputChannelForm*>())
        outputChannelForm->setAssignment(ChannelDesc[outputChannelForm->index()]);
}

/**
  Sends the channel value to the UAV to move the servo.
  Returns immediately if we are not in testing mode
  */
void ConfigOutputWidget::sendChannelTest(int index, int value)
{
    if (!m_config->channelOutTest->isChecked())
        return;

    if (index < 0 || static_cast<unsigned>(index) >= ActuatorCommand::CHANNEL_NUMELEM)
        return;

    ActuatorCommand *actuatorCommand = ActuatorCommand::getInstance(getObjectManager());
    if (!actuatorCommand)
        return;
    ActuatorCommand::DataFields actuatorCommandFields = actuatorCommand->getData();
    actuatorCommandFields.Channel[index] = value;
    actuatorCommand->setData(actuatorCommandFields);
}

bool showOutputChannelSelectWindow(bool (&selectedChannels)[ActuatorCommand::CHANNEL_NUMELEM])
{
    // Get channel descriptions
    QStringList ChannelDesc = ConfigVehicleTypeWidget::getChannelDescriptions();

    // Build up dialog
    QDialog dialog;
    QVBoxLayout layout;
    QCheckBox* checkBoxes[ActuatorCommand::CHANNEL_NUMELEM];
    QLabel infoLabel("Select output channels to calibrate: ");
    layout.addWidget(&infoLabel);
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
        checkBoxes[i] = new QCheckBox();
        checkBoxes[i]->setText(QString("Channel %0 (%1)").arg(i+1).arg(ChannelDesc[i]));
        checkBoxes[i]->setChecked(false);
        layout.addWidget(checkBoxes[i]);
    }

    QHBoxLayout horizontalLayout;
    QPushButton buttonOk("Ok");
    QPushButton buttonCancel("Cancel");
    horizontalLayout.addWidget(&buttonOk);
    horizontalLayout.addWidget(&buttonCancel);
    layout.addLayout(&horizontalLayout);

    // Connect buttons with dialog slots
    dialog.connect(&buttonOk, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(&buttonCancel, SIGNAL(clicked()), &dialog, SLOT(reject()));

    // Show dialog
    dialog.setLayout(&layout);
    if (dialog.exec() == dialog.Accepted) {
        for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++)
            selectedChannels[i] = checkBoxes[i]->isChecked();
        return true;
    }
    else
        return false;
}

/**
 * @brief ConfigOutputWidget::startESCCalibration Starts the process of ESC calibration.
 */
void ConfigOutputWidget::startESCCalibration()
{
    bool channelsToCalibrate[ActuatorCommand::CHANNEL_NUMELEM];
    if (!showOutputChannelSelectWindow(channelsToCalibrate))
        return;

    QMessageBox mbox;
    mbox.setText(QString(tr("Starting ESC calibration.<br/><b>Please remove all propellers and disconnect battery from ESCs.</b>")));
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    mbox.setDefaultButton(QMessageBox::Cancel);
    
    if (mbox.exec() != QMessageBox::Ok)
        return;

    // Get access to actuator command (for setting actual value)
    ActuatorCommand *actuatorCommand = ActuatorCommand::getInstance(getObjectManager());
    if (!actuatorCommand)
        return;
    ActuatorCommand::DataFields actuatorCommandFields = actuatorCommand->getData();
    UAVObject::Metadata mdata = actuatorCommand->getMetadata();
    // Get access to actuator settings (for min / max values)
    ActuatorSettings *actuatorSettings = ActuatorSettings::getInstance(getObjectManager());
    if (!actuatorSettings)
        return;
    ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

    // Save previous metadata
    accInitialData = mdata;

    // Change settings for period of calibration
    UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
    UAVObject::SetGcsTelemetryAcked(mdata, false);
    UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
    mdata.gcsTelemetryUpdatePeriod = 100;
    actuatorCommand->setMetadata(mdata);
    actuatorCommand->updated();

    // Increase output for all motors
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
        // Check if the output channel was selected
        if (!channelsToCalibrate[i])
            continue;

        actuatorCommandFields.Channel[i] = actuatorSettingsData.ChannelMax[i];
    }
    actuatorCommand->setData(actuatorCommandFields);

    mbox.setText(QString(tr("Motors outputs were increased to maximum. "
                            "Reconnect the battery and wait for notification from ESCs that they recognized high throttle position.<br/>"
                            "<b>Immediately after that proceed to next step.</b>")));
    if (mbox.exec() == QMessageBox::Ok) {
        // Decrease output for all motors
        for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
            // Check if the output channel was selected
            if (!channelsToCalibrate[i])
                continue;

            actuatorCommandFields.Channel[i] = actuatorSettingsData.ChannelMin[i];
        }
        actuatorCommand->setData(actuatorCommandFields);


        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.setDefaultButton(QMessageBox::Ok);
        mbox.setText(QString(tr("Motors outputs were decreased to minimum.<br/>Wait for notification from ESCs that calibration is finished.")));
        mbox.exec();
    }

    // Restore metadata
    actuatorCommand->setMetadata(accInitialData);
    actuatorCommand->updated();
}

void ConfigOutputWidget::tabSwitchingAway()
{
    stopTests();
    ConfigTaskWidget::tabSwitchingAway();
}

void ConfigOutputWidget::stopTests()
{
    if (m_config->channelOutTest->isChecked()) {
        qDebug() << "Output testing stopped by signal of incompatible mode";
    }

    m_config->channelOutTest->setChecked(false);
}

void ConfigOutputWidget::reloadBanks()
{
    Q_ASSERT(utilMngr);
    if (!utilMngr) {
        qWarning() << "Failed to get UAVObjectUtilManager";
        return;
    }

    Core::IBoardType *board = utilMngr->getBoardType();

    for (OutputChannelForm *form : findChildren<OutputChannelForm *>()) {
        int bankIndex = board->getBankFromOutputChannel(form->index() + 1);
        OutputBankForm *bankForm = outputBanks.at(bankIndex);
        // first remove each widget from it's previous bank, then add to new one
        // (doing it this way forces the order to be correct if a different board is connected with different banks)
        m_config->channelLayout->addWidget(form);
        bankForm->addChannelWidget(form);
    }

    emit channelBanksUpdated();
}
