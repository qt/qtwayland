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
        qwaylandxdgdialogv1.cpp qwaylandxdgdialogv1_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
    ATTRIBUTION_FILE_DIR_PATHS
        ../../../3rdparty/protocol/xdg-decoration
        ../../../3rdparty/protocol/xdg-shell
        ../../../3rdparty/protocol/xdg-activation
        ../../../3rdparty/protocol/xdg-foreign
        ../../../3rdparty/protocol/xdg-dialog
)

qt6_generate_wayland_protocol_client_sources(QWaylandXdgShellIntegrationPlugin
    PRIVATE_CODE
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-decoration/xdg-decoration-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-shell/xdg-shell.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-activation/xdg-activation-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-foreign/xdg-foreign-unstable-v2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-dialog/xdg-dialog-v1.xml
)

#### Keys ignored in scope 1:.:.:xdg-shell.pro:<TRUE>:
# OTHER_FILES = "xdg-shell.json"

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandXdgShellIntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
