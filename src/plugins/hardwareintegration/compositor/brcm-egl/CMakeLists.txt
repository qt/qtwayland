# Generated from brcm-egl.pro.

#####################################################################
## qt-wayland-compositor-brcm-egl Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-brcm-egl
    TYPE wayland-graphics-integration-server
    CLASS_NAME QWaylandBrcmClientBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.h
        ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/brcm-egl
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-brcm-egl
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/brcm-egl/../../../extensions/brcm.xml
)

#### Keys ignored in scope 1:.:.:brcm-egl.pro:<TRUE>:
# OTHER_FILES = "brcm-egl.json"
