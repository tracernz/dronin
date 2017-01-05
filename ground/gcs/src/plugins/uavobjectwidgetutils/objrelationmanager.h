#ifndef OBJRELATIONMANAGER_H
#define OBJRELATIONMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include "objrelation.h"
#include "uavobjectfield.h"
#include "uavobjectwidgetutils_global.h"

class UAVOBJECTWIDGETUTILS_EXPORT ObjRelationManager : public QObject
{
    Q_OBJECT
public:
    explicit ObjRelationManager(QObject *parent = Q_NULLPTR);

    void addRelation(QSharedPointer<ObjRelation> relation);

private:
    QHash<QString, QHash<QString, QList<QWeakPointer<ObjRelation>>>> relations;
    QList<QPointer<QWidget>> widgets;

private slots:
    void fieldUpdated(QSharedPointer<UAVObjectField> field);
};

#endif // OBJRELATIONMANAGER_H
