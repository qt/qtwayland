QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += linux-dmabuf.json

SOURCES += \
    main.cpp \

TARGET = qt-wayland-compositor-linux-dmabuf-unstable-v1

include(../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linux-dmabuf-unstable-v1.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = QWaylandDmabufClientBufferIntegrationPlugin
load(qt_plugin)
