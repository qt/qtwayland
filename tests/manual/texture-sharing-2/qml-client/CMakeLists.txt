# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(qml-client LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(qml-client
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Quick
)

# Resources:
set(qml-client_resource_files
    "main.qml"
)

qt_internal_add_resource(qml-client "qml-client"
    PREFIX
        "/"
    FILES
        ${qml-client_resource_files}
)
