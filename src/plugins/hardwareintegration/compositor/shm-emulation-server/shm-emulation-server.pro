QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += shm-emulation-server.json

SOURCES += \
    main.cpp

TARGET = qt-wayland-compositor-shm-emulation-server

include(../../../../hardwareintegration/compositor/shm-emulation-server/shm-emulation-server.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = ShmServerBufferIntegrationPlugin
load(qt_plugin)
