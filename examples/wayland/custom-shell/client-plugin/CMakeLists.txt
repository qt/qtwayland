# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
project(exampleshellplugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/custom-shell/plugins")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WaylandClient)

qt_add_plugin(exampleshellplugin)
target_sources(exampleshellplugin PRIVATE
    main.cpp
    exampleshellintegration.cpp exampleshellintegration.h
    examplesurface.cpp examplesurface.h
)

qt6_generate_wayland_protocol_client_sources(exampleshellplugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/example-shell.xml
)

set_target_properties(exampleshellplugin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../plugins/wayland-shell-integration"
)

target_link_libraries(exampleshellplugin PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::WaylandClient
    Qt::WaylandClientPrivate
    Wayland::Client
)

install(TARGETS exampleshellplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
