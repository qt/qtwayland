# Generated from vulkan-server.pro.

#####################################################################
## vulkan-server Plugin:
#####################################################################

qt_add_plugin(vulkan-server
    TYPE wayland-graphics-integration-client
    CLASS_NAME VulkanServerBufferPlugin
    SOURCES
        ../../../../hardwareintegration/client/vulkan-server/vulkanserverbufferintegration.cpp ../../../../hardwareintegration/client/vulkan-server/vulkanserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/vulkan-server
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(vulkan-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:vulkan-server.pro:<TRUE>:
# OTHER_FILES = "vulkan-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
