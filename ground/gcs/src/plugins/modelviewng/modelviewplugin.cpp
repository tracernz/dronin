/**
 ******************************************************************************
 * @file       modelviewplugin.cpp
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelView Model View Gadget
 * @{
 * @brief Provides a 3D model view
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

#include "modelviewplugin.h"
#include <QDebug>
#include <QtPlugin>
#include "coreplugin/icore.h"

ModelViewPlugin::ModelViewPlugin()
{

}

ModelViewPlugin::~ModelViewPlugin()
{
    Core::ICore::instance()->saveSettings(this);
}

void ModelViewPlugin::readConfig(QSettings *qSettings, Core::UAVConfigInfo *configInfo)
{
    Q_UNUSED(configInfo)
    Q_UNUSED(qSettings)
    /*qSettings->beginGroup(QLatin1String("UsageStatistics"));
    sendUsageStats = (qSettings->value(QLatin1String("SendUsageStats"), sendUsageStats).toBool());
    sendPrivateData = (qSettings->value(QLatin1String("SendPrivateData"), sendPrivateData).toBool());
    //Check the Installation UUID and Generate a new one if required
    installationUUID = QUuid(qSettings->value(QLatin1String("InstallationUUID"), "").toString());
    if (installationUUID.isNull()) { //Create new UUID
        installationUUID = QUuid::createUuid();
    }
    debugLogLevel = (qSettings->value(QLatin1String("DebugLogLevel"), debugLogLevel).toInt());

    qSettings->endGroup();*/
}

void ModelViewPlugin::saveConfig(QSettings *qSettings, Core::UAVConfigInfo *configInfo)
{
    Q_UNUSED(configInfo)
    Q_UNUSED(qSettings)
    /*
    qSettings->beginGroup(QLatin1String("UsageStatistics"));
    qSettings->setValue(QLatin1String("SendUsageStats"), sendUsageStats);
    qSettings->setValue(QLatin1String("SendPrivateData"), sendPrivateData);
    qSettings->setValue(QLatin1String("InstallationUUID"), installationUUID.toString());
    qSettings->setValue(QLatin1String("DebugLogLevel"), debugLogLevel);

    qSettings->endGroup();*/
}

bool ModelViewPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)
    Core::ICore::instance()->readSettings(this);
    return true;
}

void ModelViewPlugin::updateSettings()
{
    Core::ICore::instance()->saveSettings(this);
}

/**
 * @}
 * @}
 */
