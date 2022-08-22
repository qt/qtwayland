# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from drm-egl-server.pro.

#####################################################################
## DrmEglServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(DrmEglServerBufferPlugin
    OUTPUT_NAME drm-egl-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/client/drm-egl-server/drmeglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/drm-egl-server
    LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(DrmEglServerBufferPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:drm-egl-server.pro:<TRUE>:
# OTHER_FILES = "drm-egl-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
