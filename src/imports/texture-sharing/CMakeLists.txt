# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from texture-sharing.pro.

#####################################################################
## qwaylandtexturesharing Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandTextureSharing
    URI "QtWayland.Client.TextureSharing"
    VERSION "1.${PROJECT_VERSION_MINOR}"
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    PLUGIN_TARGET WaylandTextureSharing
    # no qmltypes file available, so nothing to install
    CLASS_NAME QWaylandTextureSharingPlugin
    SOURCES
        plugin.cpp
        sharedtextureprovider.cpp sharedtextureprovider_p.h
        texturesharingextension.cpp texturesharingextension_p.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::OpenGL
        Qt::Qml
        Qt::QuickPrivate
        Qt::WaylandClient
        Qt::WaylandClientPrivate
    PRIVATE_HEADER_FILTERS
        "^qwayland-.*\.h|^wayland-.*-protocol\.h"
)

qt6_generate_wayland_protocol_client_sources(WaylandTextureSharing
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../extensions/qt-texture-sharing-unstable-v1.xml
)

qt_internal_add_autogen_sync_header_dependencies(WaylandTextureSharing WaylandCompositor)

#### Keys ignored in scope 1:.:.:texture-sharing.pro:<TRUE>:
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWayland/Client/TextureSharing"
