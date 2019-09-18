# Generated from shm-emulation-server.pro.

#####################################################################
## qt-wayland-compositor-shm-emulation-server Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-shm-emulation-server
    TYPE wayland-graphics-integration-server
    CLASS_NAME ShmServerBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/shm-emulation-server
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-shm-emulation-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/shm-emulation-server/../../../extensions/shm-emulation-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:shm-emulation-server.pro:<TRUE>:
# OTHER_FILES = "shm-emulation-server.json"
