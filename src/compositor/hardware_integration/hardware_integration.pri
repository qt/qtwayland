QT += core

isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    HEADERS += \
        $$PWD/graphicshardwareintegration.h

    SOURCES += \
        $$PWD/graphicshardwareintegration.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}

HEADERS += \
    hardware_integration/graphicshardwareintegrationplugin.h \
    hardware_integration/graphicshardwareintegrationfactory.h

SOURCES += \
    hardware_integration/graphicshardwareintegrationplugin.cpp \
    hardware_integration/graphicshardwareintegrationfactory.cpp

