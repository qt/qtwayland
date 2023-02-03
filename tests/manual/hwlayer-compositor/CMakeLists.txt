# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(hwlayer-compositor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/hwlayer-compositor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(hwlayer-compositor
    main.cpp
)

set_target_properties(hwlayer-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(hwlayer-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(hwlayer-compositor_resource_files
    "main.qml"
)

qt6_add_resources(hwlayer-compositor "hwlayer-compositor"
    PREFIX
        "/"
    FILES
        ${hwlayer-compositor_resource_files}
)

install(TARGETS hwlayer-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
