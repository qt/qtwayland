# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from wayland-egl.pro.

#####################################################################
## QWaylandEglClientBufferPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandEglClientBufferPlugin
    OUTPUT_NAME qt-plugin-wayland-egl
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Qt::WaylandEglClientHwIntegrationPrivate
)

#### Keys ignored in scope 1:.:.:wayland-egl.pro:<TRUE>:
# OTHER_FILES = "wayland-egl.json"
