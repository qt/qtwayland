# Generated from drm-egl-server.pro.

#####################################################################
## drm-egl-server Plugin:
#####################################################################

qt_add_plugin(drm-egl-server
    TYPE wayland-graphics-integration-client
    CLASS_NAME DrmEglServerBufferPlugin
    SOURCES
        ../../../../hardwareintegration/client/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/client/drm-egl-server/drmeglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/drm-egl-server
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(drm-egl-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:drm-egl-server.pro:<TRUE>:
# OTHER_FILES = "drm-egl-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
