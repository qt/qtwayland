INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += egl wayland-server wayland-egl

SOURCES += \
    $$PWD/waylandeglclientbufferintegration.cpp

HEADERS += \
    $$PWD/waylandeglclientbufferintegration_p.h
