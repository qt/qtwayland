# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(multi-screen LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/multi-screen")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(multi-screen
    main.cpp
)

set_target_properties(multi-screen PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(multi-screen PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(multi-screen_resource_files
    "qml/Chrome.qml"
    "qml/CompositorScreen.qml"
    "qml/main.qml"
)

qt6_add_resources(multi-screen "multi-screen"
    PREFIX
        "/"
    FILES
        ${multi-screen_resource_files}
)

install(TARGETS multi-screen
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
