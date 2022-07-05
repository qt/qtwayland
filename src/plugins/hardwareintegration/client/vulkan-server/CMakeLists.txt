# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from vulkan-server.pro.

#####################################################################
## VulkanServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VulkanServerBufferPlugin
    OUTPUT_NAME vulkan-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/vulkan-server/vulkanserverbufferintegration.cpp ../../../../hardwareintegration/client/vulkan-server/vulkanserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/vulkan-server
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(VulkanServerBufferPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:vulkan-server.pro:<TRUE>:
# OTHER_FILES = "vulkan-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
