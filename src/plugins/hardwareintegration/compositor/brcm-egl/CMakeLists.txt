# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from brcm-egl.pro.

#####################################################################
## QWaylandBrcmClientBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandBrcmClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-brcm-egl
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.h
        ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/brcm-egl
    LIBRARIES
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

qt6_generate_wayland_protocol_server_sources(QWaylandBrcmClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/brcm-egl/../../../extensions/brcm.xml
)

#### Keys ignored in scope 1:.:.:brcm-egl.pro:<TRUE>:
# OTHER_FILES = "brcm-egl.json"
