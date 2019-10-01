QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += libhybris-egl-server.json

SOURCES += \
    main.cpp

TARGET = qt-wayland-compositor-libybris-egl-server.json

include(../../../../hardwareintegration/compositor/libhybris-egl-server/libhybris-egl-server.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = LibHybrisEglServerBufferIntegrationPlugin
load(qt_plugin)
