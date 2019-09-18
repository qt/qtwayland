# Generated from vulkan-server.pro.

#####################################################################
## qt-wayland-compositor-vulkan-server Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-vulkan-server
    TYPE wayland-graphics-integration-server
    CLASS_NAME VulkanServerBufferIntegrationPlugin
    SOURCES
        ../../../../hardwareintegration/compositor/vulkan-server/vulkanserverbufferintegration.cpp ../../../../hardwareintegration/compositor/vulkan-server/vulkanserverbufferintegration.h
        ../../../../hardwareintegration/compositor/vulkan-server/vulkanwrapper.cpp ../../../../hardwareintegration/compositor/vulkan-server/vulkanwrapper.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/vulkan-server
        ../../../../hardwareintegration/compositor/vulkan-server/../../../3rdparty/util
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::VulkanSupportPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::VulkanSupport
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-vulkan-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:vulkan-server.pro:<TRUE>:
# OTHER_FILES = "vulkan-server.json"
