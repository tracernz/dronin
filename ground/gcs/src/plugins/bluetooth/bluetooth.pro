TEMPLATE = lib
TARGET = Bluetooth
include(../../gcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

QT += bluetooth

OTHER_FILES += Bluetooth.pluginspec

HEADERS += \
    bluetoothplugin.h

SOURCES += \
    bluetoothplugin.cpp
