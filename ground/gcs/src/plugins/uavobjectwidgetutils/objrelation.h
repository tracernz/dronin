#ifndef OBJRELATION_H
#define OBJRELATION_H

#include <QObject>
#include <QPointer>
#include <QSharedPointer>

#include "uavobjectfield.h"
#include "uavobjectwidgetutils_global.h"

class ObjRelationManager;

class UAVOBJECTWIDGETUTILS_EXPORT ObjRelation : public QObject
{
    Q_OBJECT
    friend class ObjRelationManager;
public:
    explicit ObjRelation(QWidget *widget, const QString &objName, const QString &fieldName);
    bool valid();

private:
    QWidget* widget;
    QString objName;
    QString fieldName;
    QWeakPointer<UAVObjectField> field;
    bool m_valid;
};

#endif // OBJRELATION_H
