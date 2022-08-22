# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from linux-dmabuf-unstable-v1.pro.

#####################################################################
## QWaylandDmabufClientBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandDmabufClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-linux-dmabuf-unstable-v1
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.h
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1
    LIBRARIES
        EGL::EGL
        Libdrm::Libdrm
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

qt6_generate_wayland_protocol_server_sources(QWaylandDmabufClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/../../../3rdparty/protocol/linux-dmabuf-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:linux-dmabuf-unstable-v1.pro:<TRUE>:
# OTHER_FILES = "linux-dmabuf.json"
