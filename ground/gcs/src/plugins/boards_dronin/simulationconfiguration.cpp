/**
 ******************************************************************************
 * @file       simulationconfiguration.cpp
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Boards_dRonin dRonin board support plugin
 * @{
 * @brief Supports dRonin board configuration
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
#include "simulationconfiguration.h"
#include "ui_simulationconfiguration.h"

#include "hwsimulation.h"

SimulationConfiguration::SimulationConfiguration(QWidget *parent) :
    ConfigTaskWidget(parent),
    ui(new Ui::SimulationConfiguration)
{
    ui->setupUi(this);

    UAVObjectManager *objManager = getObjectManager();
    if (!objManager)
        return;
    auto hwSim = objManager->getObject(HwSimulation::NAME);
    if (!hwSim) {
        Q_ASSERT(false);
        qWarning() << "Invalid object! HwSimulation";
        return;
    }
    connect(hwSim.data(), &UAVObject::objectUpdated, this, &SimulationConfiguration::onLedStateUpdated);
}

SimulationConfiguration::~SimulationConfiguration()
{
    delete ui;
}

void SimulationConfiguration::onLedStateUpdated(QSharedPointer<UAVObject> obj)
{
    auto hwSim = obj.dynamicCast<HwSimulation>();
    if (!hwSim) {
        Q_ASSERT(false);
        qWarning() << "Invalid object!";
        return;
    }

    if (hwSim->getLedState_Heartbeat() == HwSimulation::LEDSTATE_ON)
        ui->lblHeartbeatLed->setPixmap(QPixmap(":/dronin/images/led_heartbeat_on.png"));
    else
        ui->lblHeartbeatLed->setPixmap(QPixmap(":/dronin/images/led_heartbeat_off.png"));
    if (hwSim->getLedState_Alarm() == HwSimulation::LEDSTATE_ON)
        ui->lblAlarmLed->setPixmap(QPixmap(":/dronin/images/led_alarm_on.png"));
    else
        ui->lblAlarmLed->setPixmap(QPixmap(":/dronin/images/led_alarm_off.png"));
}

/**
 * @}
 * @}
 */
