# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from custom-compositor.pro.

cmake_minimum_required(VERSION 3.16)
project(texture-sharing-custom-compositor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/texture-sharing/custom-compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS WaylandCompositor)

qt_add_executable(texture-sharing-custom-compositor
    main.cpp
)
set_target_properties(texture-sharing-custom-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(texture-sharing-custom-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::WaylandCompositorPrivate
)


# Resources:
set(compositor_resource_files
    "images/background.png"
    "images/car.ktx"
    "images/qt4.astc"
    "images/qt_logo.png"
    "qml/main.qml"
)

qt6_add_resources(texture-sharing-custom-compositor "compositor"
    PREFIX
        "/"
    FILES
        ${compositor_resource_files}
)

install(TARGETS texture-sharing-custom-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
