# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qwaylandqtshellplugin Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandCompositorQtShell
    URI "QtWayland.Compositor.QtShell"
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET WaylandCompositorQtShell
    SOURCES
        qwaylandqtshell.cpp qwaylandqtshell.h qwaylandqtshell_p.h
        qwaylandqtshellintegration.cpp qwaylandqtshellintegration_p.h
        qwaylandqtshellchrome.cpp qwaylandqtshellchrome.h qwaylandqtshellchrome_p.h
    DEPENDENCIES
        QtQuick
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::QuickPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
    PAST_MAJOR_VERSIONS 1
)

qt6_generate_wayland_protocol_server_sources(WaylandCompositorQtShell
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../extensions/qt-shell-unstable-v1.xml
)

qt_internal_add_autogen_sync_header_dependencies(WaylandCompositorQtShell WaylandCompositor)
