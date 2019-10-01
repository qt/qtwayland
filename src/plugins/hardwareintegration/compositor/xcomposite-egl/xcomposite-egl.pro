QT += waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += xcomposite-egl.json

SOURCES += \
    main.cpp

TARGET = qt-wayland-compositor-xcomposite-egl

include(../../../../hardwareintegration/compositor/xcomposite-egl/xcomposite-egl.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = QWaylandXCompositeEglClientBufferIntegrationPlugin
load(qt_plugin)
