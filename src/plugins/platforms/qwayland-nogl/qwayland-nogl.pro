PLUGIN_TYPE = platforms
load(qt_plugin)

include(../wayland_common/wayland_common.pri)

OTHER_FILES += qwayland-nogl.json

SOURCES += main.cpp
