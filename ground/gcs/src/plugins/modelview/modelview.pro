TEMPLATE = lib
TARGET = ModelViewGadget

include(../../gcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(modelview_dependencies.pri)

HEADERS += modelviewplugin.h \
    modelviewgadgetconfiguration.h \
    modelviewgadget.h \
    modelviewgadgetwidget.h \
    modelviewgadgetfactory.h \
    modelviewgadgetoptionspage.h
SOURCES += modelviewplugin.cpp \
    modelviewgadgetconfiguration.cpp \
    modelviewgadget.cpp \
    modelviewgadgetfactory.cpp \
    modelviewgadgetwidget.cpp \
    modelviewgadgetoptionspage.cpp
OTHER_FILES += ModelViewGadget.pluginspec
FORMS += modelviewoptionspage.ui

RESOURCES += \
    modelview.qrc
