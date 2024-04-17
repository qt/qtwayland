# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qwayland-generic.pro.

#####################################################################
## QWaylandIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandIntegrationPlugin
    OUTPUT_NAME qwayland-generic
    PLUGIN_TYPE platforms
    DEFAULT_IF "wayland" IN_LIST QT_QPA_PLATFORMS
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
)

#### Keys ignored in scope 1:.:.:qwayland-generic.pro:<TRUE>:
# OTHER_FILES = "qwayland-generic.json"
