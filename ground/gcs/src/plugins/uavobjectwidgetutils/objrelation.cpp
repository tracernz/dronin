#include "objrelation.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"

#include <QDebug>

ObjRelation::ObjRelation(QWidget *widget, const QString &o, const QString &f) :
    QObject(),
    widget(widget),
    objName(o),
    fieldName(f),
    field(),
    m_valid(false)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *uavoMngr = pm->getObject<UAVObjectManager>();
    field = uavoMngr->getField(objName, fieldName);
    if (!field) {
        Q_ASSERT(false);
        qWarning() << "Invalid object/field!" << objName << fieldName;
        return;
    }
    if (!widget) {
        Q_ASSERT(false);
        qWarning() << "Invalid widget!";
        return;
    }
    m_valid = true;
}

bool ObjRelation::valid() {
    return m_valid && widget && field;
}
