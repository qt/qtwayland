# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(WaylandCompositorWLShell
    URI "QtWayland.Compositor.WlShell"
    VERSION "${PROJECT_VERSION}"
    SOURCES
        qwaylandcompositorwlshell_p.h qwaylandcompositorwlshell.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
    NO_GENERATE_CPP_EXPORTS
    PAST_MAJOR_VERSIONS 1
)

qt_internal_add_autogen_sync_header_dependencies(WaylandCompositorWLShell WaylandCompositor)
