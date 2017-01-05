/**
 ******************************************************************************
 *
 * @file       uavmetaobject.cpp
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 *
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin
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

#include "uavmetaobject.h"
#include "uavobjectfield.h"

/**
 * Constructor
 */
UAVMetaObject::UAVMetaObject(quint32 objID, const QString& name, QSharedPointer<UAVObject> parent):
        UAVObject(objID, true, name)
{
    this->parent = parent;
    // Setup default metadata of metaobject (can not be changed)
    UAVObject::metadataInitialize(ownMetadata);
    // Setup fields
    QStringList modesBitField;
    modesBitField << tr("FlightReadOnly") << tr("GCSReadOnly") << tr("FlightTelemetryAcked") << tr("GCSTelemetryAcked") << tr("FlightUpdatePeriodic") << tr("FlightUpdateOnChange") << tr("GCSUpdatePeriodic") << tr("GCSUpdateOnChange");
    QList<QSharedPointer<UAVObjectField>> fields;
    fields.append(QSharedPointer<UAVObjectField>::create(tr("Modes"), tr("boolean"), UAVObjectField::BITFIELD, UAVObjectField::BIN, modesBitField, QStringList(), QList<int>()));
    fields.append(QSharedPointer<UAVObjectField>::create(tr("Flight Telemetry Update Period"), tr("ms"), UAVObjectField::UINT16, UAVObjectField::DEC, 1, QStringList(), QList<int>()));
    fields.append(QSharedPointer<UAVObjectField>::create(tr("GCS Telemetry Update Period"), tr("ms"), UAVObjectField::UINT16, UAVObjectField::DEC, 1, QStringList(), QList<int>()));
    fields.append(QSharedPointer<UAVObjectField>::create(tr("Logging Update Period"), tr("ms"), UAVObjectField::UINT16, UAVObjectField::DEC, 1, QStringList(), QList<int>()));
    // Initialize parent
    UAVObject::initialize(0);
    UAVObject::initializeFields(fields, (quint8*)&parentMetadata, sizeof(Metadata));
    // Setup metadata of parent
    parentMetadata = parent->getDefaultMetadata();
}

/**
 * Get the parent object
 */
QSharedPointer<UAVObject> UAVMetaObject::getParentObject()
{
    return parent;
}

/**
 * Set the metadata of the metaobject, this function will
 * do nothing since metaobjects have read-only metadata.
 */
void UAVMetaObject::setMetadata(const Metadata& mdata)
{
    Q_UNUSED(mdata);
    return; // can not update metaobject's metadata
}

/**
 * Get the metadata of the metaobject
 */
UAVObject::Metadata UAVMetaObject::getMetadata()
{
    return ownMetadata;
}

/**
 * Get the default metadata
 */
UAVObject::Metadata UAVMetaObject::getDefaultMetadata()
{
    return ownMetadata;
}

/**
 * Set the metadata held by the metaobject
 */
void UAVMetaObject::setData(const Metadata& mdata)
{
    parentMetadata = mdata;
    emit objectUpdatedAuto(QEnableSharedFromThis<UAVMetaObject>::sharedFromThis()); // trigger object updated event
    emit objectUpdated(QEnableSharedFromThis<UAVMetaObject>::sharedFromThis());
}

/**
 * Get the metadata held by the metaobject
 */
UAVObject::Metadata UAVMetaObject::getData()
{
    return parentMetadata;
}


