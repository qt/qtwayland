TEMPLATE = app
TARGET = qwidget-compositor
DEPENDPATH += .
INCLUDEPATH += .

# comment out the following CONFIG lines to disable DRM
#CONFIG += wayland_gl
#CONFIG += mesa_egl
#CONFIG += dri2_xcb
#CONFIG += xcomposite_glx
#CONFIG += xcomposite_egl

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

DESTDIR=$$PWD/../../bin/

include (../../src/qt-compositor/qt-compositor.pri)

# Input
SOURCES += main.cpp

CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/release-shared
MOC_DIR = .moc/release-shared
