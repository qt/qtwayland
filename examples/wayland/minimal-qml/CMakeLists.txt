# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(minimal-qml LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/minimal-qml")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(minimal-qml
    main.cpp
)

set_target_properties(minimal-qml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(minimal-qml PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(minimal-qml_resource_files
    "main.qml"
)

qt6_add_resources(minimal-qml "minimal-qml"
    PREFIX
        "/"
    FILES
        ${minimal-qml_resource_files}
)

install(TARGETS minimal-qml
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
