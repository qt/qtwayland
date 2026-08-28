# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(qmlclient LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Generated from qmlclient.pro.

#####################################################################
## qmlclient Binary:
#####################################################################

qt_internal_add_manual_test(qmlclient
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
        Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(qmlclient "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:qmlclient.pro:<TRUE>:
# TEMPLATE = "app"
