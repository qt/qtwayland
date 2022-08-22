# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from dmabuf-server.pro.

#####################################################################
## DmaBufServerBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(DmaBufServerBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-dmabuf-server-buffer
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/dmabuf-server
    LIBRARIES
        EGL::EGL
        Libdrm::Libdrm
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Server
)

qt6_generate_wayland_protocol_server_sources(DmaBufServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:dmabuf-server.pro:<TRUE>:
# OTHER_FILES = "dmabuf-server.json"
