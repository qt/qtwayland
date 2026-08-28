# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(texture-sharing-custom-compositor LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(texture-sharing-custom-compositor
    GUI
    SOURCES
        main.cpp
    LIBRARIES
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

qt_internal_add_resource(texture-sharing-custom-compositor "compositor"
    PREFIX
        "/"
    FILES
        ${compositor_resource_files}
)
