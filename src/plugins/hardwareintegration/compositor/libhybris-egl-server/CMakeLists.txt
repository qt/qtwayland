# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from libhybris-egl-server.pro.

#####################################################################
## LibHybrisEglServerBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(LibHybrisEglServerBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-libybris-egl-server.json
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/libhybris-egl-server
    LIBRARIES
        EGL::EGL
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

qt6_generate_wayland_protocol_server_sources(LibHybrisEglServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/libhybris-egl-server/../../../extensions/libhybris-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:libhybris-egl-server.pro:<TRUE>:
# OTHER_FILES = "libhybris-egl-server.json"
