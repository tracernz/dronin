#include "objrelationmanager.h"

#include <QSpinBox>
#include <QDebug>

ObjRelationManager::ObjRelationManager(QObject *parent) : QObject(parent)
{

}

void ObjRelationManager::addRelation(QSharedPointer<ObjRelation> relation)
{
    if (!relation || !relation->valid())
        return;

    if (widgets.contains(relation->widget)) {
        qWarning() << "Duplicate object relation on widget " << relation->widget->objectName();
        return;
    }

    widgets.append(relation->widget);
    relations[relation->objName][relation->fieldName].append(relation);

    connect(relation->field.data(), &UAVObjectField::fieldUpdated, this, &ObjRelationManager::fieldUpdated, Qt::UniqueConnection);
}

void ObjRelationManager::fieldUpdated(QSharedPointer<UAVObjectField> field)
{
    if (!field)
        return;
    auto obj = field->getObject();
    if (!obj)
        return;
    auto rels = relations[obj->getName()][field->getName()];
    for (QSharedPointer<ObjRelation> rel : rels) {
        if (!rel)
            continue;
        if (auto sb = qobject_cast<QSpinBox *>(rel->widget))
            sb->setValue(field->getValue().toInt());
    }
}
