/**
 ******************************************************************************
 *
 * @file       telemetrymonitor.cpp
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2014
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 *
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVTalkPlugin UAVTalk Plugin
 * @{
 * @brief The UAVTalk protocol plugin
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

#ifndef TELEMETRYMONITOR_H
#define TELEMETRYMONITOR_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include "uavobjectmanager.h"
#include "gcstelemetrystats.h"
#include "flighttelemetrystats.h"
#include "systemstats.h"
#include "telemetry.h"
#include "sessionmanaging.h"
#include <coreplugin/generalsettings.h>
#include <extensionsystem/pluginmanager.h>

class TelemetryMonitor : public QObject
{
    Q_OBJECT

public:
    struct objStruc
    {
        quint32 objID;
        quint32 instID;
    };

    TelemetryMonitor(UAVObjectManager* objMngr, Telemetry* tel, QHash<quint16, QList<objStruc> > sessions);
    ~TelemetryMonitor();
    QHash<quint16, QList<objStruc> > savedSessions() {return sessions;}
signals:
    void connected();
    void disconnected();
    void telemetryUpdated(double txRate, double rxRate);

public slots:
    void transactionCompleted(QSharedPointer<UAVObject> obj, bool success);
    void processStatsUpdates();
    void flightStatsUpdated(QSharedPointer<UAVObject> obj);
    void checkSessionObjNacked(QSharedPointer<UAVObject> obj, bool, bool);
private slots:
    void sessionObjUnpackedCB(QSharedPointer<UAVObject> obj);
    void objectRetrieveTimeoutCB();
    void sessionRetrieveTimeoutCB();
    void sessionInitialRetrieveTimeoutCB();
    void saveSession();
    void newInstanceSlot(QSharedPointer<UAVObject>);
private:
    QList<QSharedPointer<UAVDataObject>> delayedUpdate;
    enum connectionStatusEnum {CON_DISCONNECTED, CON_INITIALIZING, CON_SESSION_INITIALIZING, CON_RETRIEVING_OBJECTS, CON_CONNECTED_UNMANAGED,CON_CONNECTED_MANAGED};
    static const int STATS_UPDATE_PERIOD_MS = 1600;
    static const int STATS_CONNECT_PERIOD_MS = 350;
    static const int CONNECTION_TIMEOUT_MS = 8000;
    connectionStatusEnum connectionStatus;
    QPointer<UAVObjectManager> objMngr;
    QPointer<Telemetry> tel;
    QQueue<QSharedPointer<UAVObject>> queue;
    QSharedPointer<GCSTelemetryStats> gcsStatsObj;
    QSharedPointer<FlightTelemetryStats> flightStatsObj;
    QTimer* statsTimer;
    QTime* connectionTimer;
    QSharedPointer<SessionManaging> sessionObj;
    void startRetrievingObjects();
    void retrieveNextObject();
    quint16 sessionID;
    quint8 numberOfObjects;
    QTimer* objectRetrieveTimeout;
    QTimer* sessionRetrieveTimeout;
    QTimer* sessionInitialRetrieveTimeout;
    int retries;
    void changeObjectInstances(quint32 objID, quint32 instID, bool delayed);
    void startSessionRetrieving(QSharedPointer<UAVObject> session = QSharedPointer<UAVObject>());
    void sessionFallback();
    bool isManaged;
    QHash<quint16, QList<objStruc> > sessions;
    int sessionObjRetries;
    Core::Internal::GeneralSettings *settings;
};

#endif // TELEMETRYMONITOR_H
