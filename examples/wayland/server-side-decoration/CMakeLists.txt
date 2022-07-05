# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(server-side-decoration LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/server-side-decoration")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(server-side-decoration
    main.cpp
)

set_target_properties(server-side-decoration PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(server-side-decoration PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(server-side-decoration_resource_files
    "main.qml"
)

qt6_add_resources(server-side-decoration "server-side-decoration"
    PREFIX
        "/"
    FILES
        ${server-side-decoration_resource_files}
)

install(TARGETS server-side-decoration
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
