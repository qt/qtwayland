# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
##Client test shared components:
#####################################################################

qt_manual_moc(moc_files
    mockcompositor.h
    coreprotocol.h
    corecompositor.h
    datadevice.h
    fullscreenshellv1.h
    iviapplication.h
    textinput.h
    qttextinput.h
    xdgoutputv1.h
    xdgshell.h
)

add_library(SharedClientTest
    OBJECT
        corecompositor.cpp corecompositor.h
        coreprotocol.cpp coreprotocol.h
        datadevice.cpp datadevice.h
        fullscreenshellv1.cpp fullscreenshellv1.h
        iviapplication.cpp iviapplication.h
        mockcompositor.cpp mockcompositor.h
        textinput.cpp textinput.h
        qttextinput.cpp qttextinput.h
        xdgoutputv1.cpp xdgoutputv1.h
        xdgshell.cpp xdgshell.h
        ${moc_files}
)

qt6_generate_wayland_protocol_server_sources(SharedClientTest
    FILES
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/fullscreen-shell-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/ivi-application.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/wp-primary-selection-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/tablet-unstable-v2.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/text-input-unstable-v2.xml
        ${PROJECT_SOURCE_DIR}/src/extensions/qt-text-input-method-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/wayland.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/xdg-decoration-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/xdg-output-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/xdg-shell.xml
)

target_link_libraries(SharedClientTest
    PUBLIC
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Server
        Threads::Threads # special case
)

target_include_directories(SharedClientTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
