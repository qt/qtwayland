# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from drm-egl-server.pro.

#####################################################################
## DrmEglServerBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(DrmEglServerBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-drm-egl-server-buffer
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/drm-egl-server
    LIBRARIES
        EGL::EGL
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Server
)

qt6_generate_wayland_protocol_server_sources(DrmEglServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:drm-egl-server.pro:<TRUE>:
# OTHER_FILES = "drm-egl-server.json"
