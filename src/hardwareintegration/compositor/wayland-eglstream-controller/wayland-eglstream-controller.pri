INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += egl wayland-server wayland-egl

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../3rdparty/protocol/wl-eglstream-controller.xml

SOURCES += \
    $$PWD/waylandeglstreamintegration.cpp \
    $$PWD/waylandeglstreamcontroller.cpp

HEADERS += \
    $$PWD/waylandeglstreamintegration.h \
    $$PWD/waylandeglstreamcontroller.h
