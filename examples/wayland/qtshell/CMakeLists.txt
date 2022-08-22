# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
project(qtshell LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(qtshell
    main.cpp
)

set_target_properties(qtshell PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(qtshell PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(qtshell_resource_files
    "images/background.jpg"
    "qml/Chrome.qml"
    "qml/CompositorScreen.qml"
    "qml/main.qml"
)

qt6_add_resources(qtshell "qtshell"
    PREFIX
        "/"
    FILES
        ${qtshell_resource_files}
)
