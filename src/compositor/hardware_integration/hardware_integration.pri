QT += core

isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    HEADERS += \
        hardware_integration/qwaylandgraphicshardwareintegration.h \
        hardware_integration/qwaylandgraphicshardwareintegrationfactory.h \
        hardware_integration/qwaylandgraphicshardwareintegrationplugin.h

    SOURCES += \
        hardware_integration/qwaylandgraphicshardwareintegration.cpp \
        hardware_integration/qwaylandgraphicshardwareintegrationfactory.cpp \
        hardware_integration/qwaylandgraphicshardwareintegrationplugin.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}
