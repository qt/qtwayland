# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(spanning-screens LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/spanning-screens")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

qt_add_executable(spanning-screens
    main.cpp
)

set_target_properties(spanning-screens PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(spanning-screens PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)

# Resources:
set(spanning-screens_resource_files
    "main.qml"
)

qt6_add_resources(spanning-screens "spanning-screens"
    PREFIX
        "/"
    FILES
        ${spanning-screens_resource_files}
)

install(TARGETS spanning-screens
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
