# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from vulkan-server.pro.

#####################################################################
## VulkanServerBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VulkanServerBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-vulkan-server
    PLUGIN_TYPE wayland-graphics-integration-server
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
        Qt::OpenGLPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Server
)

qt6_generate_wayland_protocol_server_sources(VulkanServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:vulkan-server.pro:<TRUE>:
# OTHER_FILES = "vulkan-server.json"
