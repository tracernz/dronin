/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.cpp
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
#include "uavobjectmanager.h"

/**
 * Constructor
 */
UAVObjectManager::UAVObjectManager()
{
}

UAVObjectManager::~UAVObjectManager()
{
}

/**
 * Register an object with the manager. This function must be called for all newly created instances.
 * A new instance can be created directly by instantiating a new object or by calling clone() of
 * an existing object. The object will be registered and will be properly initialized so that it can accept
 * updates.
 */
bool UAVObjectManager::registerObject(QSharedPointer<UAVDataObject> obj)
{
    if (!obj)
        return false;

    // Check if this object type is already in the list
    quint32 objID = obj->getObjID();
    if (objects.contains(objID)) { //Known object ID
        if (objects.value(objID).contains(obj->getInstID()))//Instance already present
            return false;
        if (obj->isSingleInstance())
            return false;
        if (obj->getInstID() >= MAX_INSTANCES)
            return false;

        QSharedPointer<UAVDataObject> refObj = objects.value(objID).first().dynamicCast<UAVDataObject>();
        if (!refObj)
            return false;

        QSharedPointer<UAVMetaObject> mobj = refObj->getMetaObject();
        if (objects.value(objID).last()->getInstID() < obj->getInstID()) {
            //Space between last existent instance and new one, lets fill the gaps
            for (quint32 instidx = objects.value(objID).last()->getInstID() + 1;
                    instidx < obj->getInstID(); ++instidx) {
                QSharedPointer<UAVDataObject> cobj = obj->clone(instidx);
                if (!cobj)
                    return false;
                cobj->initialize(instidx,mobj);
                QMap<quint32, QSharedPointer<UAVObject>> ppp;
                ppp.insert(instidx, cobj);
                objects[objID].insert(instidx,cobj);
                getObject(cobj->getObjID())->emitNewInstance(cobj);//TODO??
                emit newInstance(cobj);
            }
        } else if (obj->getInstID() == 0) {
            obj->initialize(objects.value(objID).last()->getObjID() + 1, mobj);
        } else {
            return false;
        }
        // Add the actual object instance in the list
        objects[objID].insert(obj->getInstID(),obj);
        getObject(objID)->emitNewInstance(obj);
        emit newInstance(obj);
        return true;
    } else {
        // If this point is reached then this is the first time this object type (ID) is added in the list
        // create a new list of the instances, add in the object collection and create the object's metaobject
        // Create metaobject
        QString mname = obj->getName();
        mname.append("Meta");
        QSharedPointer<UAVMetaObject> mobj = QSharedPointer<UAVMetaObject>::create(objID + 1, mname, obj);
        // Initialize object
        obj->initialize(0, mobj);
        // Add to list
        addObject(obj);
        addObject(mobj);
        return true;
    }
 }

/**
 * @brief unregisters an object instance and all instances bigger than the one passed as argument from the manager
 * @param obj pointer to the object to unregister
 * @return false if object is single instance
 */
bool UAVObjectManager::unRegisterObject(QSharedPointer<UAVDataObject> obj)
{
    // Check if this object type is already in the list
    quint32 objID = obj->getObjID();
    if (obj->isSingleInstance())
        return false;
    quint32 instances = static_cast<quint32>(objects.value(obj->getObjID()).count());
    for (quint32 x = obj->getInstID(); x < instances; ++x) {
        getObject(objects.value(objID).value(x)->getObjID())->emitInstanceRemoved(objects.value(objID).value(x));
        emit instanceRemoved(objects.value(objID).value(x));
        objects[objID].remove(x);
    }
    return true;
}

void UAVObjectManager::addObject(QSharedPointer<UAVObject> obj)
{
    // Add to list
    QMap<quint32, QSharedPointer<UAVObject>> list;
    list.insert(obj->getInstID(), obj);
    objects.insert(obj->getObjID(), list);

    objectsByName.insert(obj->getName(), list);

    emit newObject(obj);
}

/**
 * Get all objects. A two dimentional QVector is returned. Objects are grouped by
 * instances of the same object type.
 */
QVector<QVector<QSharedPointer<UAVObject>>> UAVObjectManager::getObjectsVector()
{
    QVector<QVector<QSharedPointer<UAVObject>>> vector;
    for (const ObjectMap &map : objects.values())
        vector.append(map.values().toVector());
    return vector;
}

QHash<quint32, UAVObjectManager::ObjectMap> UAVObjectManager::getObjects()
{
    return objects;
}

/**
 * Same as getObjects() but will only return DataObjects.
 */
QVector<QVector<QSharedPointer<UAVDataObject>>> UAVObjectManager::getDataObjectsVector()
{
    QVector<QVector<QSharedPointer<UAVDataObject>>> vector;
    for (const ObjectMap &map : objects.values()) {
        if (map.first().dynamicCast<UAVDataObject>()) {
            QVector<QSharedPointer<UAVDataObject>> vec;
            for (QSharedPointer<UAVObject> o : map) {
                if(auto dobj = o.dynamicCast<UAVDataObject>())
                    vec.append(dobj);
            }
            vector.append(vec);
        }
    }
    return vector;
}

/**
 * Same as getObjects() but will only return MetaObjects.
 */
QVector<QVector<QSharedPointer<UAVMetaObject>>> UAVObjectManager::getMetaObjectsVector()
{
    QVector< QVector<QSharedPointer<UAVMetaObject>> > vector;
    for (const ObjectMap &map : objects.values()) {
        if (auto obj = map.first().dynamicCast<UAVMetaObject>()) {
            QVector<QSharedPointer<UAVMetaObject>> vec;
            for (const QSharedPointer<UAVObject> o : map) {
                if(auto mobj = o.dynamicCast<UAVMetaObject>())
                    vec.append(mobj);
            }
            vector.append(vec);
        }
     }
    return vector;
}

/**
 * Get a specific object given its name and instance ID
 * @returns The object is found or NULL if not
 */
QSharedPointer<UAVObject> UAVObjectManager::getObject(const QString& name, quint32 instId)
{
    return getObject(name, 0, instId);
}

/**
 * Get a specific object given its object and instance ID
 * @returns The object is found or NULL if not
 */
QSharedPointer<UAVObject> UAVObjectManager::getObject(quint32 objId, quint32 instId)
{
    return getObject(NULL, objId, instId);
}

/**
 * Helper function for the public getObject() functions.
 */
QSharedPointer<UAVObject> UAVObjectManager::getObject(const QString& name, quint32 objId, quint32 instId)
{
    if (!name.isNull()) {
        if (objectsByName.contains(name))
            return objectsByName.value(name).value(instId);
        return QSharedPointer<UAVObject>();
    } else if(objects.contains(objId)) {
        return objects.value(objId).value(instId);
    }
    return QSharedPointer<UAVObject>();
}

/**
 * Get all the instances of the object specified by name
 */
QVector<QSharedPointer<UAVObject>> UAVObjectManager::getObjectInstancesVector(const QString& name)
{
    return getObjectInstancesVector(&name, 0);
}

/**
 * Get all the instances of the object specified by its ID
 */
QVector<QSharedPointer<UAVObject>> UAVObjectManager::getObjectInstancesVector(quint32 objId)
{
    return getObjectInstancesVector(NULL, objId);
}

/**
 * Helper function for the public getObjectInstances()
 */
QVector<QSharedPointer<UAVObject>> UAVObjectManager::getObjectInstancesVector(const QString* name, quint32 objId)
{
    if (!name->isNull()) {
        for (const ObjectMap &map : objects) {
            if(map.first()->getName().compare(name) == 0)
                return map.values().toVector();
        }
    }
    else if(objects.contains(objId))
        return objects.value(objId).values().toVector();
    return  QVector<QSharedPointer<UAVObject>>();
}

/**
 * Get the number of instances for an object given its name
 */
qint32 UAVObjectManager::getNumInstances(const QString& name)
{
    return getNumInstances(&name, 0);
}

/**
 * Get the number of instances for an object given its ID
 */
qint32 UAVObjectManager::getNumInstances(quint32 objId)
{
    return getNumInstances(NULL, objId);
}

/**
 * Helper function for public getNumInstances
 */
qint32 UAVObjectManager::getNumInstances(const QString* name, quint32 objId)
{
    if (!name->isNull()) {
        for (const ObjectMap &map : objects) {
            if(map.first()->getName().compare(name) == 0)
                return map.count();
        }
    } else if (objects.contains(objId)) {
        return objects.value(objId).count();
    }
    return -1;
}

QSharedPointer<UAVObjectField> UAVObjectManager::getField(const QString &objName, const QString &fieldName, quint32 instId)
{
    QSharedPointer<UAVObject> uavo = getObject(objName, instId);
    Q_ASSERT(uavo);
    if (!uavo)
        return QSharedPointer<UAVObjectField>();
    auto field = uavo->getField(fieldName);
    Q_ASSERT(field);
    return field;
}
