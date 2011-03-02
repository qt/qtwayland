TEMPLATE = app
TARGET = qt-compositor
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../3rdparty/wayland

# comment out the following line to disable DRM
CONFIG += wayland_drm

DESTDIR=$$PWD/../../bin/

LIBS += -L ../../lib

wayland_drm  {
  LIBS += -lxcb-dri2 -lEGL
}
include (qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
