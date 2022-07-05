# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from dmabuf-server.pro.

#####################################################################
## DmaBufServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(DmaBufServerBufferPlugin
    OUTPUT_NAME dmabuf-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/client/dmabuf-server/dmabufserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/dmabuf-server
    LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(DmaBufServerBufferPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:dmabuf-server.pro:<TRUE>:
# OTHER_FILES = "dmabuf-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
