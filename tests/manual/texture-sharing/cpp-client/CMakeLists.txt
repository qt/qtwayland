# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-client LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

include(QtWaylandSetup)
qt_internal_get_wayland_extensions_dir(extensions_dir)

# Generated from cpp-client.pro.

#####################################################################
## cpp-client Binary:
#####################################################################

qt_internal_add_manual_test(cpp-client
    GUI
    SOURCES
        ../../../../src/imports/texture-sharing/texturesharingextension.cpp
        ../../../../src/imports/texture-sharing/texturesharingextension_p.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/imports/texture-sharing
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::OpenGL
        Qt::WaylandClientPrivate
)

qt6_generate_wayland_protocol_client_sources(cpp-client
    PRIVATE_CODE
    FILES
        ${extensions_dir}/qt-texture-sharing-unstable-v1.xml
        ${extensions_dir}/server-buffer-extension.xml
)

#### Keys ignored in scope 1:.:.:cpp-client.pro:<TRUE>:
# INSTALLS = "target"
# target.path = "$$[QT_INSTALL_EXAMPLES]/wayland/texture-sharing/cpp-client"
