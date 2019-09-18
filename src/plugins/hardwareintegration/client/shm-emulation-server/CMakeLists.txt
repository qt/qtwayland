# Generated from shm-emulation-server.pro.

#####################################################################
## shm-emulation-server Plugin:
#####################################################################

qt_add_plugin(shm-emulation-server
    TYPE wayland-graphics-integration-client
    CLASS_NAME ShmServerBufferPlugin
    SOURCES
        ../../../../hardwareintegration/client/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/client/shm-emulation-server/shmserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/shm-emulation-server
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(shm-emulation-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/shm-emulation-server/../../../extensions/shm-emulation-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:shm-emulation-server.pro:<TRUE>:
# OTHER_FILES = "shm-emulation-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
