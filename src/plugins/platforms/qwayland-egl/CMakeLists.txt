# Generated from qwayland-egl.pro.

#####################################################################
## qwayland-egl Plugin:
#####################################################################

qt_add_plugin(qwayland-egl
    TYPE platforms
    CLASS_NAME QWaylandEglPlatformIntegrationPlugin
    SOURCES
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglinclude.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.h
        ../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.cpp ../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../hardwareintegration/client/wayland-egl
    LIBRARIES
        Qt::EglSupportPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::EglSupport
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
        Wayland::Egl
)

#### Keys ignored in scope 1:.:.:qwayland-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-egl.json"
