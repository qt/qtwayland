# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from xdgshell.pro.

#####################################################################
## qwaylandcompositorxdgshellplugin Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandCompositorXdgShell
    URI "QtWayland.Compositor.XdgShell"
    VERSION "${PROJECT_VERSION}"
    NO_PLUGIN_OPTIONAL
    PLUGIN_TARGET WaylandCompositorXdgShell
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    CLASS_NAME QWaylandCompositorXdgShellPlugin
    SOURCES
        qwaylandcompositorxdgshellplugin.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:xdgshell.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/XdgShell"
