TEMPLATE = app
TARGET = qwindow-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

DESTDIR=$$PWD/../../bin/

include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp \
    qopenglwindow.cpp \
    surfacerenderer.cpp \
    qwindowcompositor.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared

# Touch support
isEmpty(QT_SOURCE_TREE) {
    QTBASE = $$[QT_INSTALL_DATA]
} else {
    QTBASE = $$QT_SOURCE_TREE
}
#TOUCHSCREEN_BASE = $$QTBASE/src/plugins/generic/touchscreen
#SOURCES += $$TOUCHSCREEN_BASE/qtouchscreen.cpp
#HEADERS += $$TOUCHSCREEN_BASE/qtouchscreen.h
#INCLUDEPATH += $$TOUCHSCREEN_BASE
#LIBS += -ludev -lmtdev
QT += gui opengl

target.path += $$[QT_INSTALL_DATA]/bin
INSTALLS += target

HEADERS += \
    qopenglwindow.h \
    surfacerenderer.h \
    qwindowcompositor.h






