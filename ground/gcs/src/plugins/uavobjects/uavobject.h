/**
 ******************************************************************************
 *
 * @file       uavobject.h
 *
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef UAVOBJECT_H
#define UAVOBJECT_H

#include "uavobjects_global.h"
#include <QtGlobal>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QList>
#include <QFile>
#include <QSharedPointer>
#include <QEnableSharedFromThis>
#include <qglobal.h>
#include "uavobjectfield.h"

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#define UAVOBJ_ACCESS_SHIFT 0
#define UAVOBJ_GCS_ACCESS_SHIFT 1
#define UAVOBJ_TELEMETRY_ACKED_SHIFT 2
#define UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT 3
#define UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT 4
#define UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT 6
#define UAVOBJ_UPDATE_MODE_MASK 0x3

class UAVObjectField;

class UAVOBJECTS_EXPORT UAVObject: public QObject, private QEnableSharedFromThis<UAVObject>
{
    Q_OBJECT

public:

    /**
     * Object update mode
     */
    typedef enum {
            UPDATEMODE_MANUAL = 0,  /** Manually update object, by calling the updated() function */
            UPDATEMODE_PERIODIC = 1, /** Automatically update object at periodic intervals */
            UPDATEMODE_ONCHANGE = 2, /** Only update object when its data changes */
            UPDATEMODE_THROTTLED = 3 /** Object is updated on change, but not more often than the interval time */
    } UpdateMode;

    /**
     * Access mode
     */
    typedef enum {
            ACCESS_READWRITE = 0,
            ACCESS_READONLY = 1
    } AccessMode;

    /**
     * Object metadata, each object has a meta object that holds its metadata. The metadata define
     * properties for each object and can be used by multiple modules (e.g. telemetry and logger)
     *
     * The object metadata flags are packed into a single 16 bit integer.
     * The bits in the flag field are defined as:
     *
     *   Bit(s)  Name                       Meaning
     *   ------  ----                       -------
     *      0    access                     Defines the access level for the local transactions (readonly=0 and readwrite=1)
     *      1    gcsAccess                  Defines the access level for the local GCS transactions (readonly=0 and readwrite=1), not used in the flight s/w
     *      2    telemetryAcked             Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
     *      3    gcsTelemetryAcked          Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
     *    4-5    telemetryUpdateMode        Update mode used by the telemetry module (UAVObjUpdateMode)
     *    6-7    gcsTelemetryUpdateMode     Update mode used by the GCS (UAVObjUpdateMode)
     */
     PACK(typedef struct {
        quint8 flags; /** Defines flags for update and logging modes and whether an update should be ACK'd (bits defined above) */
        quint16 flightTelemetryUpdatePeriod; /** Update period used by the telemetry module (only if telemetry mode is PERIODIC) */
        quint16 gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
        quint16 loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */
     }) Metadata;


    UAVObject(quint32 objID, bool isSingleInst, const QString& name);
    void initialize(quint32 instID);
    quint32 getObjID();
    quint32 getInstID();
    bool isSingleInstance();
    QString getName();
    QString getCategory();
    QString getDescription();
    quint32 getNumBytes(); 
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    virtual void setMetadata(const Metadata& mdata) = 0;
    virtual Metadata getMetadata() = 0;
    virtual Metadata getDefaultMetadata() = 0;
    qint32 getNumFields();
    QList<QSharedPointer<UAVObjectField>> getFields();
    QSharedPointer<UAVObjectField> getField(const QString& name);
    QString toString();
    QString toStringBrief();
    QString toStringData();
    QJsonObject getJsonRepresentation();
    void emitTransactionCompleted(bool success);
    void emitTransactionCompleted(bool success, bool nacked);
    void emitNewInstance(QSharedPointer<UAVObject>);
    void emitInstanceRemoved(QSharedPointer<UAVObject>);

    // Metadata accessors
    static void metadataInitialize(Metadata& meta);
    static AccessMode getFlightAccess(const Metadata& meta);
    static void setFlightAccess(Metadata& meta, AccessMode mode);
    static AccessMode getGcsAccess(const Metadata& meta);
    static void setGcsAccess(Metadata& meta, AccessMode mode);
    static quint8 getFlightTelemetryAcked(const Metadata& meta);
    static void setFlightTelemetryAcked(Metadata& meta, quint8 val);
    static quint8 getGcsTelemetryAcked(const Metadata& meta);
    static void setGcsTelemetryAcked(Metadata& meta, quint8 val);
    static UpdateMode getFlightTelemetryUpdateMode(const Metadata& meta);
    static void setFlightTelemetryUpdateMode(Metadata& meta, UpdateMode val);
    static UpdateMode getGcsTelemetryUpdateMode(const Metadata& meta);
    static void setGcsTelemetryUpdateMode(Metadata& meta, UpdateMode val);
		
public slots:
    void requestUpdate();
    void requestUpdateAllInstances();
    void updated();

signals:
    /**
     * @brief Signal sent whenever any field of the object is updated
     * @param obj
     *
     * objectUpdated is emitted either when a field is updated (setData), or when
     * an "unpack" event happens, i.e. an update coming from the telemetry
     * link. Note that objects also send signals specific to all their fields separately
     * as well.
     *
     */
    void objectUpdated(QSharedPointer<UAVObject> obj);

    /**
     * @brief objectUpdatedAuto: triggered on "setData" only (Object data updated by changing the data structure)
     *
     * The telemetry manager listens to this signal, and sends updates on the telemetry
     * link.
     * @param obj
     */
    void objectUpdatedAuto(QSharedPointer<UAVObject> obj);

    /**
     * @brief objectUpdatedManual: triggered only from the "updated" slot in uavobject
     * The telemetry manager listens to this signal, and sends updates on the telemetry
     * link.
     * @param obj
     */
    void objectUpdatedManual(QSharedPointer<UAVObject> obj);

    /**
     * @brief objectUpdatedPeriodic: not used anywhere ?
     * @param obj
     */
    void objectUpdatedPeriodic(QSharedPointer<UAVObject> obj);

    /**
     * @brief objectUnpacked: triggered whenever an object is unpacked
     * (i.e. arrives from the telemetry link)
     * @param obj
     */
    void objectUnpacked(QSharedPointer<UAVObject> obj);

    /**
     * @brief updateRequested
     * @param obj
     */
    void updateRequested(QSharedPointer<UAVObject> obj);

    /**
     * @brief updateAllInstancesRequested
     * @param obj
     */
    void updateAllInstancesRequested(QSharedPointer<UAVObject> obj);
    /**
     * @brief transactionCompleted. Triggered by a call to
     * emitTransactionCompleted - done in telemetry.cpp whenever a
     * transaction finishes.
     * @param obj
     * @param success
     */
    void transactionCompleted(QSharedPointer<UAVObject> obj, bool success);
    void transactionCompleted(QSharedPointer<UAVObject> obj, bool success, bool nack);
    /**
     * @brief newInstance
     * @param obj
     */
    void newInstance(QSharedPointer<UAVObject> obj);

    /**
     * @brief instance removed from manager
     * @param obj
     */
    void instanceRemoved(QSharedPointer<UAVObject> obj);

private slots:
    void fieldUpdated(QSharedPointer<UAVObjectField> field);

protected:
    quint32 objID;
    quint32 instID;
    bool isSingleInst;
    QString name;
    QString description;
    QString category;
    quint32 numBytes;
    quint8* data;
    QList<QSharedPointer<UAVObjectField>> fields;
    void initializeFields(QList<QSharedPointer<UAVObjectField>> &fields, quint8* data, quint32 numBytes);
    void setDescription(const QString& description);
    void setCategory(const QString& category);

};

#endif // UAVOBJECT_H
