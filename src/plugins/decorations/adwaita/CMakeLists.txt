# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWaylandAdwaitaDecorationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandAdwaitaDecorationPlugin
    OUTPUT_NAME adwaita
    PLUGIN_TYPE wayland-decoration-client
    SOURCES
        main.cpp
        qwaylandadwaitadecoration.cpp
    LIBRARIES
        Qt::Core
        Qt::DBus
        Qt::Gui
        Qt::Svg
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

#### Keys ignored in scope 1:.:.:bradient.pro:<TRUE>:
# OTHER_FILES = "bradient.json"

