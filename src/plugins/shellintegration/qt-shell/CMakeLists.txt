# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWaylandQtShellIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandQtShellIntegrationPlugin
    OUTPUT_NAME qt-shell
    PLUGIN_TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandqtshellintegration.cpp qwaylandqtshellintegration.h
        qwaylandqtsurface.cpp qwaylandqtsurface_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandQtShellIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../extensions/qt-shell-unstable-v1.xml
)

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandQtShellIntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
