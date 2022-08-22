# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from wayland-egl.pro.

#####################################################################
## QWaylandEglClientBufferIntegrationPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandEglClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-wayland-egl
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Qt::WaylandEglCompositorHwIntegrationPrivate
)

#### Keys ignored in scope 1:.:.:wayland-egl.pro:<TRUE>:
# OTHER_FILES = "wayland-egl.json"
