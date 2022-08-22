# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(overview-compositor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/overview-compositor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(overview-compositor
    main.cpp
)

set_target_properties(overview-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(overview-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(overview-compositor_resource_files
    "main.qml"
)

qt6_add_resources(overview-compositor "overview-compositor"
    PREFIX
        "/"
    FILES
        ${overview-compositor_resource_files}
)

install(TARGETS overview-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
