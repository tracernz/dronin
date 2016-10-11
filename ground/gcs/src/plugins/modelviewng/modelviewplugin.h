/**
 ******************************************************************************
 * @file       modelviewplugin.h
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

#ifndef USAGESTATSPLUGIN_H_
#define USAGESTATSPLUGIN_H_

#include <coreplugin/iconfigurableplugin.h>
#include <extensionsystem/iplugin.h>

class ModelViewPlugin :  public Core::IConfigurablePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.dronin.plugins.ModelView")

public:
    ModelViewPlugin();
    ~ModelViewPlugin();
    void readConfig(QSettings *qSettings, Core::UAVConfigInfo *configInfo);
    void saveConfig(QSettings *qSettings, Core::UAVConfigInfo *configInfo);
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized() {};

public slots:
    void updateSettings();
};

#endif /* USAGESTATSPLUGIN_H_ */

/**
 * @}
 * @}
 */
