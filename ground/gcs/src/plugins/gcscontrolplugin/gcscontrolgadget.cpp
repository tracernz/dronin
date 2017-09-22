/**
 ******************************************************************************
 *
 * @file       GCSControlgadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "gcscontrol.h"
#include "gcscontrolgadget.h"
#include "gcscontrolgadgetwidget.h"
#include "gcscontrolgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QDebug>

#define JOYSTICK_UPDATE_RATE 50

GCSControlGadget::GCSControlGadget(QString classId, GCSControlGadgetWidget *widget, QWidget *parent)
    : IUAVGadget(classId, parent)
    , m_widget(widget)
    , controlsMode(0)
{
    connect(getManualControlCommand(), &UAVObject::objectUpdated, this,
            &GCSControlGadget::manualControlCommandUpdated);
    connect(widget, &GCSControlGadgetWidget::sticksChanged, this,
            &GCSControlGadget::sticksChangedLocally);
    connect(widget, &GCSControlGadgetWidget::controlEnabled, this,
            &GCSControlGadget::enableControl);
    connect(this, &GCSControlGadget::sticksChangedRemotely, widget,
            &GCSControlGadgetWidget::updateSticks);
    connect(widget, &GCSControlGadgetWidget::flightModeChangedLocaly, this,
            &GCSControlGadget::flightModeChanged);

    manualControlCommandUpdated(getManualControlCommand());

    joystickTime.start();
}

GCSControlGadget::~GCSControlGadget()
{
    delete m_widget;
}

void GCSControlGadget::loadConfiguration(IUAVGadgetConfiguration *config)
{
    GCSControlGadgetConfiguration *GCSControlConfig =
        qobject_cast<GCSControlGadgetConfiguration *>(config);

    QList<int> ql = GCSControlConfig->getChannelsMapping();
    rollChannel = ql.at(0);
    pitchChannel = ql.at(1);
    yawChannel = ql.at(2);
    throttleChannel = ql.at(3);

    controlsMode = GCSControlConfig->getControlsMode();
    gcsReceiverMode = GCSControlConfig->getGcsReceiverMode();
    m_widget->allowGcsControl(gcsReceiverMode);

    for (unsigned int i = 0; i < 8; i++) {
        buttonSettings[i].ActionID = GCSControlConfig->getbuttonSettings(i).ActionID;
        buttonSettings[i].FunctionID = GCSControlConfig->getbuttonSettings(i).FunctionID;
        buttonSettings[i].Amount = GCSControlConfig->getbuttonSettings(i).Amount;
        buttonSettings[i].Amount = GCSControlConfig->getbuttonSettings(i).Amount;
        channelReverse[i] = GCSControlConfig->getChannelsReverse().at(i);
    }
}

/**
 * @brief GCSControlGadget::enableControl Enable or disable sending updates
 * In the case of GCSReceiver mode it enables the timer for updates
 * @param enable Whether to enable or disable it
 */
void GCSControlGadget::enableControl(bool enable)
{
    enableSending = enable;
    if (enableSending)
        getGcsControl()->beginGCSControl();
    else
        getGcsControl()->endGCSControl();
}

ManualControlCommand *GCSControlGadget::getManualControlCommand()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    return dynamic_cast<ManualControlCommand *>(
        objManager->getObject(QString("ManualControlCommand")));
}

GCSControl *GCSControlGadget::getGcsControl()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    GCSControl *gcsControl = pm->getObject<GCSControl>();
    Q_ASSERT(gcsControl);
    return gcsControl;
}

void GCSControlGadget::manualControlCommandUpdated(UAVObject *obj)
{

    // Not sending then show updates from transmitter
    if (enableSending)
        return;

    double roll = obj->getField("Roll")->getDouble();
    double pitch = obj->getField("Pitch")->getDouble();
    double yaw = obj->getField("Yaw")->getDouble();
    double throttle = obj->getField("Throttle")->getDouble();
    // Remap RPYT to left X/Y and right X/Y depending on mode
    switch (controlsMode) {
    case 1:
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        emit sticksChangedRemotely(yaw, -pitch, roll, throttle);
        break;
    case 2:
        // Mode 2: LeftX = Yaw, LeftY = Throttle, RightX = Roll, RightY = Pitch
        emit sticksChangedRemotely(yaw, throttle, roll, -pitch);
        break;
    case 3:
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        emit sticksChangedRemotely(roll, -pitch, yaw, throttle);
        break;
    case 4:
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        emit sticksChangedRemotely(roll, throttle, yaw, -pitch);
        break;
    }
}

/**
  Update the manual commands - maps depending on mode
  */
void GCSControlGadget::sticksChangedLocally(double leftX, double leftY, double rightX,
                                            double rightY)
{
    if (enableSending)
        setGcsReceiver(leftX, leftY, rightX, rightY);
}

//! Set the GCS Receiver object
void GCSControlGadget::setGcsReceiver(double leftX, double leftY, double rightX, double rightY)
{
    GCSControl *ctr = getGcsControl();
    Q_ASSERT(ctr);
    if (!ctr)
        return;

    double newRoll = 0;
    double newPitch = 0;
    double newYaw = 0;
    double newThrottle = 0;

    // Remap left X/Y and right X/Y to RPYT depending on mode
    switch (controlsMode) {
    case 1:
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        newRoll = rightX;
        newPitch = -leftY;
        newYaw = leftX;
        newThrottle = rightY;
        break;
    case 2:
        // Mode 2: LeftX = Yaw, LeftY = Throttle, RightX = Roll, RightY = Pitch
        newRoll = rightX;
        newPitch = -rightY;
        newYaw = leftX;
        newThrottle = leftY;
        break;
    case 3:
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        newRoll = leftX;
        newPitch = -leftY;
        newYaw = rightX;
        newThrottle = rightY;
        break;
    case 4:
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        newRoll = leftX;
        newPitch = -rightY;
        newYaw = rightX;
        newThrottle = leftY;
        break;
    }
    ctr->setThrottle(newThrottle);
    ctr->setRoll(newRoll);
    ctr->setPitch(newPitch);
    ctr->setYaw(newYaw);

    switch (controlsMode) {
    case 1:
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        emit sticksChangedRemotely(newYaw, -newPitch, newRoll, newThrottle);
        break;
    case 2:
        // Mode 2: LeftX = Yaw, LeftY = Throttle, RightX = Roll, RightY = Pitch
        emit sticksChangedRemotely(newYaw, newThrottle, newRoll, -newPitch);
        break;
    case 3:
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        emit sticksChangedRemotely(newRoll, -newPitch, newYaw, newThrottle);
        break;
    case 4:
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        emit sticksChangedRemotely(newRoll, newThrottle, newYaw, -newPitch);
        break;
    }
}

void GCSControlGadget::flightModeChanged(ManualControlSettings::FlightModePositionOptions mode)
{
    if (enableSending)
        getGcsControl()->setFlightMode(mode);
}

void GCSControlGadget::gamepads(quint8 count)
{
    Q_UNUSED(count);
}
