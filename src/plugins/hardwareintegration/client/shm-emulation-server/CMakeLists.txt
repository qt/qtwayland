# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from shm-emulation-server.pro.

#####################################################################
## ShmServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(ShmServerBufferPlugin
    OUTPUT_NAME shm-emulation-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/client/shm-emulation-server/shmserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/shm-emulation-server
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt6_generate_wayland_protocol_client_sources(ShmServerBufferPlugin
    PRIVATE_CODE
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../extensions/server-buffer-extension.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../extensions/shm-emulation-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:shm-emulation-server.pro:<TRUE>:
# OTHER_FILES = "shm-emulation-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
