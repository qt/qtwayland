# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(custom-extension-qml-client)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/custom-extension/qml-client")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WaylandClient)

qt_add_executable(custom-extension-qml-client
    ../client-common/customextension.cpp ../client-common/customextension.h
    main.cpp
)

qt6_generate_wayland_protocol_client_sources(custom-extension-qml-client
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/custom.xml
)

set_target_properties(custom-extension-qml-client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(custom-extension-qml-client PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Qml
    Qt::Quick
    Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(custom-extension-qml-client "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS custom-extension-qml-client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
