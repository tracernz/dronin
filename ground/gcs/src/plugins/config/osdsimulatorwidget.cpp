/**
 ******************************************************************************
 * @file       osdsimulatorwidget.cpp
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup [Group]
 * @{
 * @addtogroup %CLASS%
 * @{
 * @brief [Brief]
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

#include "osdsimulatorwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <utils/svgimageprovider.h>
#include <utils/pathutils.h>
#include <QDebug>
#include <QSvgRenderer>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QMouseEvent>

#include <QQmlEngine>
#include <QQmlContext>
#include <QWidget>
#include "stabilizationdesired.h"
#include "onscreendisplaysettings.h"
#include "onscreendisplaypagesettings.h"

OsdSimulatorWidget::OsdSimulatorWidget() :
    QQuickView()
{
    setResizeMode(SizeRootObjectToView);

    objectsToExport << "PositionActual" <<
                       "BaroAltitude" <<
                       "FlightStatus" <<
                       "AttitudeActual" <<
                       "FlightBatteryState" <<
                       "VelocityActual" <<
                       "SystemStats" <<
                       "Accels" <<
                       "GPSPosition" <<
                       "ManualControlCommand" <<
                       "GPSVelocity" <<
                       "AirspeedActual" <<
                       "OnScreenDisplayPageSettings" <<
                       "OnScreenDisplayPageSettings2" <<
                       "OnScreenDisplayPageSettings3" <<
                       "OnScreenDisplayPageSettings4" <<
                       "OnScreenDisplaySettings" <<
                       "TabletInfo";

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_objManager = pm->getObject<UAVObjectManager>();

    foreach (const QString &objectName, objectsToExport) {
        exportUAVOInstance(objectName, 0);
    }

    //to expose settings values
    engine()->rootContext()->setContextProperty("qmlWidget", this);

    QString fn = Utils::PathUtils().InsertDataPath(QString("%%DATAPATH%%/osdsimulator/OsdSimulator.qml"));
    setQmlFile(fn);
}

OsdSimulatorWidget::~OsdSimulatorWidget()
{
}


/**
 * @brief OsdSimulatorWidget::exportUAVOInstance Makes the UAVO available inside the QML. This works via the Q_PROPERTY()
 * values in the UAVO synthetic-headers
 * @param objectName UAVObject name
 * @param instId Instance ID
 */
void OsdSimulatorWidget::exportUAVOInstance(const QString &objectName, int instId)
{
    UAVObject* object = m_objManager->getObject(objectName, instId);
    if (object) {
        engine()->rootContext()->setContextProperty(objectName, object);
    } else {
        qWarning() << "[OsdSimulator] Failed to load object" << objectName;
    }
}


/**
 * @brief OsdSimulatorWidget::resetUAVOExport Makes the UAVO no longer available inside the QML.
 * @param objectName UAVObject name
 * @param instId Instance ID
 */
void OsdSimulatorWidget::resetUAVOExport(const QString &objectName, int instId)
{
    UAVObject* object = m_objManager->getObject(objectName, instId);
    if (object)
        engine()->rootContext()->setContextProperty(objectName, (QObject*)NULL);
    else
        qWarning() << "Failed to load object" << objectName;
}

void OsdSimulatorWidget::setQmlFile(QString fn)
{
    m_qmlFileName = fn;

    engine()->removeImageProvider("svg");
    SvgImageProvider *svgProvider = new SvgImageProvider(fn);
    engine()->addImageProvider("svg", svgProvider);

    engine()->clearComponentCache();

    //it's necessary to allow qml side to query svg element position
    engine()->rootContext()->setContextProperty("svgRenderer", svgProvider);
    engine()->setBaseUrl(QUrl::fromLocalFile(fn));

    setSource(QUrl::fromLocalFile(fn));

    foreach(const QQmlError &error, errors()) {
        qDebug() << error.description();
    }
}

void OsdSimulatorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // Reload the schene on the middle mouse button click.
    if (event->button() == Qt::MiddleButton) {
        setQmlFile(m_qmlFileName);
    }

    QQuickView::mouseReleaseEvent(event);
}
