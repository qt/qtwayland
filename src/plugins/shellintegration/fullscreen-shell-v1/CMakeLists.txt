# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from fullscreen-shell-v1.pro.

#####################################################################
## QWaylandFullScreenShellV1IntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandFullScreenShellV1IntegrationPlugin
    OUTPUT_NAME fullscreen-shell-v1
    PLUGIN_TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandfullscreenshellv1integration.cpp qwaylandfullscreenshellv1integration.h
        qwaylandfullscreenshellv1surface.cpp qwaylandfullscreenshellv1surface.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandFullScreenShellV1IntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/fullscreen-shell-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:fullscreen-shell-v1.pro:<TRUE>:
# OTHER_FILES = "fullscreen-shell-v1.json"
