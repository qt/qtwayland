TEMPLATE = app
TARGET = qt-compositor
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../3rdparty/wayland

DESTDIR=$$PWD/../../bin/


LIBS += -L ../../lib

LIBS += -lxcb-dri2 -lEGL
include (qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
