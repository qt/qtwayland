TEMPLATE = app
TARGET = qt-compositor
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../src/qt-compositor/

# comment out the following line to disable DRM
CONFIG += wayland_drm

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib

include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
