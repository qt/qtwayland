# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(compositor)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/server-buffer/compositor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml WaylandCompositor)

qt_add_executable(compositor
    main.cpp
    sharebufferextension.cpp sharebufferextension.h
)

qt6_generate_wayland_protocol_server_sources(compositor
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../share-buffer.xml
)

set_target_properties(compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::WaylandCompositorPrivate
)

# Resources:
set(compositor_resource_files
    "images/Siberischer_tiger_de_edit02.jpg"
    "images/background.png"
    "qml/main.qml"
)

qt6_add_resources(compositor "compositor"
    PREFIX
        "/"
    FILES
        ${compositor_resource_files}
)

install(TARGETS compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
