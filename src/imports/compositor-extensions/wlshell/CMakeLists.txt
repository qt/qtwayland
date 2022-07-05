# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from wlshell.pro.

#####################################################################
## qwaylandcompositorwlshellplugin Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandCompositorWLShell
    URI "QtWayland.Compositor.WlShell"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QWaylandCompositorWlShellPlugin
    NO_PLUGIN_OPTIONAL
    PLUGIN_TARGET WaylandCompositorWLShell
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        qwaylandcompositorwlshellplugin.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:wlshell.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/WlShell"
