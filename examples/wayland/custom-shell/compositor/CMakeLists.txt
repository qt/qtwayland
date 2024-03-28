# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
project(custom-shell-compositor)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/custom-shell/compositor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml WaylandCompositor)

qt_add_executable(custom-shell-compositor
    exampleshell.cpp exampleshell.h
    exampleshellintegration.cpp exampleshellintegration.h
    main.cpp
)

qt6_generate_wayland_protocol_server_sources(custom-shell-compositor
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/example-shell.xml
)

set_target_properties(custom-shell-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(custom-shell-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::WaylandCompositor
)

# Resources:
set(compositor_resource_files
    "images/background.png"
    "qml/main.qml"
)

qt6_add_resources(custom-shell-compositor "compositor"
    PREFIX
        "/"
    FILES
        ${compositor_resource_files}
)

install(TARGETS custom-shell-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
