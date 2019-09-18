# Generated from dmabuf-server.pro.

#####################################################################
## qt-wayland-compositor-dmabuf-server-buffer Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-dmabuf-server-buffer
    TYPE wayland-graphics-integration-server
    CLASS_NAME DmaBufServerBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/dmabuf-server
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

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-dmabuf-server-buffer
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:dmabuf-server.pro:<TRUE>:
# OTHER_FILES = "dmabuf-server.json"
