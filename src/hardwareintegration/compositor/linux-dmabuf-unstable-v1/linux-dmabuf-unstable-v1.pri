INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += egl drm wayland-server

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../3rdparty/protocol/linux-dmabuf-unstable-v1.xml

QT += egl_support-private

SOURCES += \
    $$PWD/linuxdmabufclientbufferintegration.cpp \
    $$PWD/linuxdmabuf.cpp

HEADERS += \
    $$PWD/linuxdmabufclientbufferintegration.h \
    $$PWD/linuxdmabuf.h
