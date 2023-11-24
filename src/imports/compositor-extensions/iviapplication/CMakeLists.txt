# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from iviapplication.pro.

#####################################################################
## qwaylandcompositoriviapplicationplugin Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandCompositorIviapplication
    URI "QtWayland.Compositor.IviApplication"
    VERSION "${PROJECT_VERSION}"
    SOURCES
        qwaylandcompositoriviapplicationforeign.cpp qwaylandcompositoriviapplicationforeign_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
    NO_GENERATE_CPP_EXPORTS
    PAST_MAJOR_VERSIONS 1
)

qt_internal_add_autogen_sync_header_dependencies(WaylandCompositorIviapplication WaylandCompositor)

#### Keys ignored in scope 1:.:.:iviapplication.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/IviApplication"
