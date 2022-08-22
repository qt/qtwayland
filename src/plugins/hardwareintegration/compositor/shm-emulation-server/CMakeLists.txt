# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from shm-emulation-server.pro.

#####################################################################
## ShmServerBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(ShmServerBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-shm-emulation-server
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/shm-emulation-server
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Server
)

qt6_generate_wayland_protocol_server_sources(ShmServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/shm-emulation-server/../../../extensions/shm-emulation-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:shm-emulation-server.pro:<TRUE>:
# OTHER_FILES = "shm-emulation-server.json"
