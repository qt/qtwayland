# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from xdg-shell.pro.

#####################################################################
## QWaylandXdgShellIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandXdgShellIntegrationPlugin
    OUTPUT_NAME xdg-shell
    PLUGIN_TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandxdgdecorationv1.cpp qwaylandxdgdecorationv1_p.h
        qwaylandxdgactivationv1.cpp qwaylandxdgactivationv1_p.h
        qwaylandxdgshell.cpp qwaylandxdgshell_p.h
        qwaylandxdgshellintegration.cpp qwaylandxdgshellintegration_p.h
        qwaylandxdgexporterv2.cpp qwaylandxdgexporterv2_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandXdgShellIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-decoration-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-shell.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-activation-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-foreign-unstable-v2.xml
)

#### Keys ignored in scope 1:.:.:xdg-shell.pro:<TRUE>:
# OTHER_FILES = "xdg-shell.json"

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandXdgShellIntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
