# Generated from linux-dmabuf-unstable-v1.pro.

#####################################################################
## qt-wayland-compositor-linux-dmabuf-unstable-v1 Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-linux-dmabuf-unstable-v1
    TYPE wayland-graphics-integration-server
    CLASS_NAME QWaylandDmabufClientBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.h
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1
    LIBRARIES
        EGL::EGL
        Qt::CorePrivate
        Qt::EglSupportPrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::EglSupport
        Qt::Gui
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-linux-dmabuf-unstable-v1
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/../../../3rdparty/protocol/linux-dmabuf-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:linux-dmabuf-unstable-v1.pro:<TRUE>:
# OTHER_FILES = "linux-dmabuf.json"
