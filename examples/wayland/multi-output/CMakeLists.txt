# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(multi-output LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/multi-output")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(multi-output
    main.cpp
)

set_target_properties(multi-output PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(multi-output PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(multi-output_resource_files
    "images/background.jpg"
    "qml/GridScreen.qml"
    "qml/ShellChrome.qml"
    "qml/ShellScreen.qml"
    "qml/main.qml"
)

qt6_add_resources(multi-output "multi-output"
    PREFIX
        "/"
    FILES
        ${multi-output_resource_files}
)

install(TARGETS multi-output
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
