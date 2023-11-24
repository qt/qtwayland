# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qwaylandcompositorpresentationtimeplugin Plugin:
#####################################################################

# Note: INTERFACE_AUTOMOC_MACRO_NAMES is a CMake 3.27 feature, so readd it here for older CMake versions.
list(APPEND CMAKE_AUTOMOC_MACRO_NAMES
        "Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT"
        "Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS"
        "Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS"
        "Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS")

qt_internal_add_qml_module(WaylandCompositorPresentationTime
    URI "QtWayland.Compositor.PresentationTime"
    VERSION "${PROJECT_VERSION}"
    SOURCES
        qwaylandcompositorpresentationtimeforeign.cpp qwaylandcompositorpresentationtimeforeign_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositorPrivate
    NO_GENERATE_CPP_EXPORTS
    PAST_MAJOR_VERSIONS 1
)

qt_internal_add_autogen_sync_header_dependencies(WaylandCompositorPresentationTime
    WaylandCompositor)
