# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(custom-extension-cpp-client)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/custom-extension/cpp-client")

find_package(Qt6 REQUIRED COMPONENTS Core Gui WaylandClient)

qt_add_executable(custom-extension-cpp-client
    ../client-common/customextension.cpp ../client-common/customextension.h
    main.cpp
)

qt6_generate_wayland_protocol_client_sources(custom-extension-cpp-client
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/custom.xml
)

set_target_properties(custom-extension-cpp-client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(custom-extension-cpp-client PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::WaylandClient
)

install(TARGETS custom-extension-cpp-client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
