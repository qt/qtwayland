QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += linux-dmabuf.json

SOURCES += \
    main.cpp \

include(../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linux-dmabuf-unstable-v1.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = QWaylandDmabufClientBufferIntegrationPlugin
load(qt_plugin)
