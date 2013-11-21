PLUGIN_TYPE = wayland-graphics-integration
load(qt_plugin)

QT = compositor compositor-private core-private gui-private

OTHER_FILES += wayland_egl.json

LIBS += -lEGL

SOURCES += \
    main.cpp

include($PWD/../../../../hardwareintegration/compositor/brcm-egl/brcm-egl.pri)

OTHER_FILES += brcm-egl.json

