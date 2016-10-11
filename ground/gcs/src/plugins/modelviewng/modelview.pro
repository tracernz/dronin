TEMPLATE = lib
TARGET = ModelView

QT += 3dcore 3drender 3dinput 3dquick qml quick

include(../../gcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += MODELVIEW_LIBRARY

HEADERS += \
	modelviewplugin.h

SOURCES += \
	modelviewplugin.cpp

OTHER_FILES += \
	modelview.pluginspec
