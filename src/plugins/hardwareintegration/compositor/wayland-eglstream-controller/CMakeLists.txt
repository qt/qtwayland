# Generated from wayland-eglstream-controller.pro.

#####################################################################
## qt-wayland-compositor-wayland-eglstream-controller Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-wayland-eglstream-controller
    TYPE wayland-graphics-integration-server
    CLASS_NAME QWaylandEglStreamBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.h
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller
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

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-wayland-eglstream-controller
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/wayland-eglstream-controller/../../../3rdparty/protocol/wl-eglstream-controller.xml
)

#### Keys ignored in scope 1:.:.:wayland-eglstream-controller.pro:<TRUE>:
# OTHER_FILES = "wayland-eglstream-controller.json"
