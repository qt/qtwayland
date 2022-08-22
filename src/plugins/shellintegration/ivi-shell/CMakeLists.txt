# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from ivi-shell.pro.

#####################################################################
## QWaylandIviShellIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandIviShellIntegrationPlugin
    OUTPUT_NAME ivi-shell
    PLUGIN_TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandivishellintegration.cpp qwaylandivishellintegration.h
        qwaylandivisurface.cpp qwaylandivisurface_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandIviShellIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/ivi-application.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/ivi-controller.xml
)

#### Keys ignored in scope 1:.:.:ivi-shell.pro:<TRUE>:
# OTHER_FILES = "ivi-shell.json"

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandIviShellIntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
