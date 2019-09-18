# Generated from drm-egl-server.pro.

#####################################################################
## qt-wayland-compositor-drm-egl-server-buffer Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-drm-egl-server-buffer
    TYPE wayland-graphics-integration-server
    CLASS_NAME DrmEglServerBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/drm-egl-server
    LIBRARIES
        EGL::EGL
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-drm-egl-server-buffer
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:drm-egl-server.pro:<TRUE>:
# OTHER_FILES = "drm-egl-server.json"
