TEMPLATE = app
TARGET = qt-compositor
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../src/qt-compositor/

# comment out the following CONFIG lines to disable DRM
CONFIG += wayland_gl
CONFIG += mesa_egl

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib

include (../../src/qt-compositor/qt-compositor.pri)

LIBS += -L/home/jlind/install/lib
INCLUDEPATH += /home/jlind/install/include

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
