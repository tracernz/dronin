TOOLS_DIR = $$PWD/../tools

# If the PYTHON environment variable isn't set (by Make)
# then we set it ourselves.
PYTHON_LOCAL = $$(PYTHON)

isEmpty(PYTHON_LOCAL) {
    unix: PYTHON_LOCAL = python2
    win32: PYTHON_LOCAL = python
    macx: PYTHON_LOCAL = python
}

# use ccache with gcc and clang in debug config
CONFIG(debug, debug|release):*-g++*|*-clang* {
    CCACHE = $$(CCACHE_BIN)
    isEmpty(CCACHE) { # use ccache with Qt Creator
        CCACHE = $$system(which ccache 2>/dev/null)
    }
    !isEmpty(CCACHE) {
        QMAKE_CC=$$CCACHE $$QMAKE_CC
        QMAKE_CXX=$$CCACHE $$QMAKE_CXX
    }
}

DR_QT_MAJOR_VERSION=5
DR_QT_MINOR_VERSION=8
DR_QT_PATCH_VERSION=0

BREAKPAD = $$TOOLS_DIR/breakpad/20170129
