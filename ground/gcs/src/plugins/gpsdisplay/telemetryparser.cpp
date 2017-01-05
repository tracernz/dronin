/**
 ******************************************************************************
 *
 * @file       telemetryparser.cpp
 * @author     Edouard Lafargue & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration
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


#include "telemetryparser.h"
#include <math.h>
#include <QDebug>
#include <QStringList>


/**
 * Initialize the parser
 */
TelemetryParser::TelemetryParser(QObject *parent) : GPSParser(parent)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    if (auto gpsObj = objManager->getObject("GPSPosition").dynamicCast<UAVDataObject>())
        connect(gpsObj.data(), &UAVObject::objectUpdated, this, &TelemetryParser::updateGPS);
    else
        qWarning() << "Error: Object is unknown (GPSPosition).";

    if (auto gpsObj = objManager->getObject("GPSTime").dynamicCast<UAVDataObject>())
        connect(gpsObj.data(), &UAVObject::objectUpdated, this, &TelemetryParser::updateTime);
    else
        qWarning() << "Error: Object is unknown (GPSTime).";

    if (auto gpsObj = objManager->getObject("GPSSatellites").dynamicCast<UAVDataObject>())
        connect(gpsObj.data(), &UAVObject::objectUpdated, this, &TelemetryParser::updateSats);
    else
        qWarning() << "Error: Object is unknown (GPSSatellites).";

}

TelemetryParser::~TelemetryParser()
{

}


void TelemetryParser::updateGPS(QSharedPointer<UAVObject> object1) {
    auto field = object1->getField(QString("Satellites"));
    Q_ASSERT(field);
    if(!field)
        return;

    emit sv(field->getValue().toInt());

    double lat = object1->getField(QString("Latitude"))->getDouble();
    double lon = object1->getField(QString("Longitude"))->getDouble();
    double alt = object1->getField(QString("Altitude"))->getDouble();
    lat *= 1E-7;
    lon *= 1E-7;
    emit position(lat,lon,alt);

    double hdg = object1->getField(QString("Heading"))->getDouble();
    double spd = object1->getField(QString("Groundspeed"))->getDouble();
    emit speedheading(spd,hdg);

    QString fix = object1->getField(QString("Status"))->getValue().toString();
    emit fixtype(fix);

    double hdop = object1->getField(QString("HDOP"))->getDouble();
    double vdop = object1->getField(QString("VDOP"))->getDouble();
    double pdop = object1->getField(QString("PDOP"))->getDouble();
    emit dop(hdop,vdop,pdop);


}

void TelemetryParser::updateTime(QSharedPointer<UAVObject> object1) {
    double hour = object1->getField(QString("Hour"))->getDouble();
    double minute = object1->getField(QString("Minute"))->getDouble();
    double second = object1->getField(QString("Second"))->getDouble();
    double time = second + minute*100 + hour*10000;
    double year = object1->getField(QString("Year"))->getDouble();
    double month = object1->getField(QString("Month"))->getDouble();
    double day = object1->getField(QString("Day"))->getDouble();
    double date = day + month * 100 + year * 10000;
    emit datetime(date,time);
}

/**
  Updates the satellite constellation.

  Not really optimized for now, we should only send updates for the stas
  which have changed instead of updating everything... That said, Qt is supposed
  to be able to optimize redraws anyway.
  */
void TelemetryParser::updateSats(QSharedPointer<UAVObject> object1) {
    auto prn = object1->getField(QString("PRN"));
    auto elevation = object1->getField(QString("Elevation"));
    auto azimuth = object1->getField(QString("Azimuth"));
    auto snr = object1->getField(QString("SNR"));

    for (unsigned int i=0;i< prn->getNumElements();i++) {
        emit satellite(i,prn->getValue(i).toInt(),elevation->getValue(i).toInt(),
                       azimuth->getValue(i).toInt(), snr->getValue(i).toInt());
    }

}
