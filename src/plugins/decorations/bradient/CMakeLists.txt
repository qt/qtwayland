# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from bradient.pro.

#####################################################################
## QWaylandBradientDecorationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandBradientDecorationPlugin
    OUTPUT_NAME bradient
    PLUGIN_TYPE wayland-decoration-client
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

#### Keys ignored in scope 1:.:.:bradient.pro:<TRUE>:
# OTHER_FILES = "bradient.json"
