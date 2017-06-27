QT += xml
QT += widgets
TEMPLATE = lib
TARGET = PathPlanner
DEFINES += PATHPLANNER_LIBRARY

include(../../gcsplugin.pri)

include(../../libs/utils/utils.pri)

include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += pathplannergadget.h \
    waypointdialog.h \
    waypointdelegate.h

HEADERS += pathplanner_global.h
HEADERS += pathplannergadgetwidget.h
HEADERS += pathplannergadgetfactory.h
HEADERS += pathplannerplugin.h
HEADERS += flightdatamodel.h
HEADERS += modeluavoproxy.h
HEADERS += ipathalgorithm.h
HEADERS += algorithms/pathfillet.h

SOURCES += pathplannergadget.cpp \
    waypointdialog.cpp \
    waypointdelegate.cpp \
    ipathalgorithm.cpp
SOURCES += pathplannergadgetwidget.cpp
SOURCES += pathplannergadgetfactory.cpp
SOURCES += pathplannerplugin.cpp
SOURCES += flightdatamodel.cpp
SOURCES += modeluavoproxy.cpp
SOURCES += algorithms/pathfillet.cpp

OTHER_FILES += PathPlanner.pluginspec

FORMS += pathplanner.ui
FORMS += waypoint_dialog.ui

RESOURCES += pathplanner.qrc
