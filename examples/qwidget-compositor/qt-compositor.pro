TEMPLATE = app
TARGET = qt-compositor
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../src/qt-compositor/

# comment out the following line to disable DRM
CONFIG += wayland_egl

wayland_egl {
    QT += opengl
}

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
