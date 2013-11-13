QT += core

isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    HEADERS += \
        hardware_integration/qwaylandclientbufferintegration.h \
        hardware_integration/qwaylandclientbufferintegrationfactory.h \
        hardware_integration/qwaylandclientbufferintegrationplugin.h

    SOURCES += \
        hardware_integration/qwaylandclientbufferintegration.cpp \
        hardware_integration/qwaylandclientbufferintegrationfactory.cpp \
        hardware_integration/qwaylandclientbufferintegrationplugin.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}
