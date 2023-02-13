# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(fancy-compositor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/fancy-compositor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(fancy-compositor
    main.cpp
)

set_target_properties(fancy-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(fancy-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(fancy-compositor_resource_files
    "images/background.jpg"
    "qml/Chrome.qml"
    "qml/CompositorScreen.qml"
    "qml/Keyboard.qml"
    "qml/main.qml"
)

qt6_add_resources(fancy-compositor "fancy-compositor"
    PREFIX
        "/"
    FILES
        ${fancy-compositor_resource_files}
)

install(TARGETS fancy-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
