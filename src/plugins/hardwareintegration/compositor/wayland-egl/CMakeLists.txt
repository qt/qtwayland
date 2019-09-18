# Generated from wayland-egl.pro.

#####################################################################
## qt-wayland-compositor-wayland-egl Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-wayland-egl
    TYPE wayland-graphics-integration-server
    CLASS_NAME QWaylandEglClientBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration.cpp ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/wayland-egl
    LIBRARIES
        EGL::EGL
        Qt::CorePrivate
        Qt::EglSupportPrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Egl
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::EglSupport
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:wayland-egl.pro:<TRUE>:
# OTHER_FILES = "wayland-egl.json"
