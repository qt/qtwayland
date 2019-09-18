# Generated from dmabuf-server.pro.

#####################################################################
## dmabuf-server Plugin:
#####################################################################

qt_add_plugin(dmabuf-server
    TYPE wayland-graphics-integration-client
    CLASS_NAME DmaBufServerBufferPlugin
    SOURCES
        ../../../../hardwareintegration/client/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/client/dmabuf-server/dmabufserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/dmabuf-server
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(dmabuf-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:dmabuf-server.pro:<TRUE>:
# OTHER_FILES = "dmabuf-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
