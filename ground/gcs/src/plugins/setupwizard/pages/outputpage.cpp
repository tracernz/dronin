/**
 ******************************************************************************
 *
 * @file       outputpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @see        The GNU Public License (GPL) Version 3
 *
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SetupWizard Setup Wizard
 * @{
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
 */

#include "outputpage.h"
#include "ui_outputpage.h"
#include "setupwizard.h"

OutputPage::OutputPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),

    ui(new Ui::OutputPage)
{
    ui->setupUi(this);
}

OutputPage::~OutputPage()
{
    delete ui;
}

//! Set output ranges
void OutputPage::setOutputRanges(ActuatorUtils::ActuatorType type)
{
    QList<actuatorChannelSettings> allSettings = getWizard()->getActuatorSettings();
    for (int i = 0; i < allSettings.count(); i++) {
        actuatorChannelSettings settings = allSettings[i];
        settings.channelMin     = ActuatorUtils::minPulse(type) + 0.5;
        settings.channelNeutral = ActuatorUtils::neutralPulse(type) + 0.5;
        settings.channelMax     = ActuatorUtils::maxPulse(type);
        allSettings[i] = settings;
    }
    getWizard()->setActuatorSettings(allSettings);
}

bool OutputPage::validatePage()
{
    ActuatorUtils::ActuatorType type;
    if (ui->oneShot125Button->isChecked())
        type = ActuatorUtils::TYPE_ONESHOT125;
    else if (ui->oneShot42Button->isChecked())
        type = ActuatorUtils::TYPE_ONESHOT42;
    else
        type = ActuatorUtils::TYPE_PWMESC;

    getWizard()->setESCType(type);
    setOutputRanges(type);

    return true;
}
