isEmpty(QT_WAYLAND_GL_CONFIG):QT_WAYLAND_GL_CONFIG = $$(QT_WAYLAND_GL_CONFIG)

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    CONFIG += wayland-scanner
    WAYLANDSERVERSOURCES += \
        ../extensions/server-buffer-extension.xml \
        ../extensions/hardware-integration.xml

    HEADERS += \
        hardware_integration/qwaylandclientbufferintegration.h \
        hardware_integration/qwaylandclientbufferintegrationfactory.h \
        hardware_integration/qwaylandclientbufferintegrationplugin.h \
        hardware_integration/qwaylandserverbufferintegration.h \
        hardware_integration/qwaylandserverbufferintegrationfactory.h \
        hardware_integration/qwaylandserverbufferintegrationplugin.h \
        hardware_integration/qwlhwintegration_p.h

    SOURCES += \
        hardware_integration/qwaylandclientbufferintegration.cpp \
        hardware_integration/qwaylandclientbufferintegrationfactory.cpp \
        hardware_integration/qwaylandclientbufferintegrationplugin.cpp \
        hardware_integration/qwaylandserverbufferintegration.cpp \
        hardware_integration/qwaylandserverbufferintegrationfactory.cpp \
        hardware_integration/qwaylandserverbufferintegrationplugin.cpp \
        hardware_integration/qwlhwintegration.cpp

    DEFINES += QT_COMPOSITOR_WAYLAND_GL
} else {
    system(echo "Qt-Compositor configured as raster only compositor")
}
