/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.h
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2014
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2015-2016
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

#ifndef UAVOBJECTMANAGER_H
#define UAVOBJECTMANAGER_H

#include "uavobjects_global.h"
#include "uavobject.h"
#include "uavdataobject.h"
#include "uavmetaobject.h"
#include <QVector>
#include <QHash>
#include <QDebug>

class UAVOBJECTS_EXPORT UAVObjectManager: public QObject
{
    Q_OBJECT

public:
    UAVObjectManager();
    ~UAVObjectManager();
    typedef QMap<quint32,UAVObject*> ObjectMap;
    bool registerObject(UAVDataObject* obj);
    QVector< QVector<UAVObject*> > getObjectsVector();
    QHash<quint32, QMap<quint32,UAVObject*> > getObjects();
    QVector< QVector<UAVDataObject*> > getDataObjectsVector();
    QVector< QVector<UAVMetaObject*> > getMetaObjectsVector();

    /**
     * Get a specific object given its name and instance ID and check it is valid
     * @returns The object is found or null pointer if not
     */
    template<typename T = UAVObject>
    T *getRequiredObject(const QString &name, quint32 instId = 0) {
        T *obj = qobject_cast<T *>(getObject(name, 0, instId));
        if (!obj) {
            qWarning() << "Could not get UAVO " << name;
            Q_ASSERT(false);
            return Q_NULLPTR;
        }
        return obj;
    }
    /**
     * Get a specific object given its name and instance ID
     * @returns The object is found or null pointer if not
     */
    template<typename T = UAVObject>
    T *getObject(const QString &name, quint32 instId = 0) {
        T *obj = qobject_cast<T *>(getObject(name, 0, instId));
        if (!obj)
            return Q_NULLPTR;
        return obj;
    }
    /**
     * Get a specific object given its object and instance ID
     * @returns The object is found or null pointer if not
     */
    template<typename T = UAVObject>
    T *getRequiredObject(quint32 objId, quint32 instId = 0) {
        T *obj = qobject_cast<T *>(getObject("", objId, instId));
        if (!obj) {
            qWarning() << "Could not get UAVO " << objId;
            Q_ASSERT(false);
            return Q_NULLPTR;
        }
        return obj;
    }
    /**
     * Get a specific object given its object and instance ID
     * @returns The object is found or null pointer if not
     */
    template<typename T = UAVObject>
    T *getObject(quint32 objId, quint32 instId = 0) {
        T *obj = qobject_cast<T *>(getObject("", objId, instId));
        if (!obj)
            return Q_NULLPTR;
        return obj;
    }

    QVector<UAVObject*> getObjectInstancesVector(const QString& name);
    QVector<UAVObject*> getObjectInstancesVector(quint32 objId);
    qint32 getNumInstances(const QString& name);
    qint32 getNumInstances(quint32 objId);    
    bool unRegisterObject(UAVDataObject *obj);
signals:
    void newObject(UAVObject* obj);
    void newInstance(UAVObject* obj);
    void instanceRemoved(UAVObject* obj);
private:
    static const quint32 MAX_INSTANCES = 1000;
    QHash<quint32, QMap<quint32,UAVObject*> > objects;
    QHash<QString, QMap<quint32,UAVObject*> > objectsByName;

    void addObject(UAVObject* obj);
    UAVObject* getObject(const QString& name, quint32 objId, quint32 instId);
    QVector<UAVObject*> getObjectInstancesVector(const QString* name, quint32 objId);
    qint32 getNumInstances(const QString* name, quint32 objId);
};


#endif // UAVOBJECTMANAGER_H
