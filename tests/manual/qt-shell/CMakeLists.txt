# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
project(qt-shell LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(qt-shell
    main.cpp
)
set_target_properties(qt-shell PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(qt-shell PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(qt-shell_resource_files
    "images/background.jpg"
    "qml/Chrome.qml"
    "qml/HandleHandler.qml"
    "qml/CompositorScreen.qml"
    "qml/Keyboard.qml"
    "qml/main.qml"
)

qt6_add_resources(qt-shell "qt-shell"
    PREFIX
        "/"
    FILES
        ${qt-shell_resource_files}
)
