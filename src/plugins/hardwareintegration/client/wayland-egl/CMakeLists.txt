# Generated from wayland-egl.pro.

#####################################################################
## QWaylandEglClientBufferPlugin Plugin:
#####################################################################

qt_add_plugin(QWaylandEglClientBufferPlugin
    OUTPUT_NAME qt-plugin-wayland-egl
    TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.cpp ../../../../hardwareintegration/client/wayland-egl/qwaylandeglclientbufferintegration.h
        ../../../../hardwareintegration/client/wayland-egl/qwaylandeglinclude.h
        ../../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.cpp ../../../../hardwareintegration/client/wayland-egl/qwaylandeglwindow.h
        ../../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.cpp ../../../../hardwareintegration/client/wayland-egl/qwaylandglcontext.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/wayland-egl
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::EglSupportPrivate
        Qt::Gui
        Qt::OpenGLPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
        Wayland::Egl
)

#### Keys ignored in scope 1:.:.:wayland-egl.pro:<TRUE>:
# OTHER_FILES = "wayland-egl.json"
