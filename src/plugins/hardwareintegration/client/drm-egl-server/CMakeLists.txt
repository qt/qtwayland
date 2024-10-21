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
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt6_generate_wayland_protocol_client_sources(DrmEglServerBufferPlugin
    PRIVATE_CODE
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../extensions/drm-egl-server-buffer.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../extensions/server-buffer-extension.xml
)

#### Keys ignored in scope 1:.:.:drm-egl-server.pro:<TRUE>:
# OTHER_FILES = "drm-egl-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
