PLUGIN_TYPE = wayland-graphics-integration-client
load(qt_plugin)

QT += waylandclient-private

include(../../../../hardwareintegration/client/brcm-egl/brcm-egl.pri)

OTHER_FILES += \
    brcm-egl.json

SOURCES += main.cpp

