TEMPLATE  = subdirs
CONFIG   += ordered
QT += widgets
SUBDIRS   = \
    qscispinbox\
    aggregation \
    extensionsystem \
    utils \
    tlmapcontrol \
    qwt \
    libcrashreporter-qt
win32 {
SUBDIRS   += \
    zlib
}
SUBDIRS   += \
    quazip
SDL {
SUBDIRS += sdlgamepad
}

SUBDIRS +=
