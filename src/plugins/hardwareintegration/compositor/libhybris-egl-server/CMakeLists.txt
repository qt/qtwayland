# Generated from libhybris-egl-server.pro.

#####################################################################
## qt-wayland-compositor-libybris-egl-server.json Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-libybris-egl-server.json
    TYPE wayland-graphics-integration-server
    CLASS_NAME LibHybrisEglServerBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/libhybris-egl-server
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

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-libybris-egl-server.json
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/libhybris-egl-server/../../../extensions/libhybris-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:libhybris-egl-server.pro:<TRUE>:
# OTHER_FILES = "libhybris-egl-server.json"
