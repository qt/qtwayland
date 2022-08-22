# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from compositor.pro.

#####################################################################
## tst_compositor Test:
#####################################################################

qt_internal_add_test(tst_compositor
    SOURCES
        mockclient.cpp mockclient.h
        mockkeyboard.cpp mockkeyboard.h
        mockpointer.cpp mockpointer.h
        mockseat.cpp mockseat.h
        mockxdgoutputv1.cpp mockxdgoutputv1.h
        testcompositor.cpp testcompositor.h
        testkeyboardgrabber.cpp testkeyboardgrabber.h
        testseat.cpp testseat.h
        tst_compositor.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Client
        Wayland::Server
)

qt6_generate_wayland_protocol_client_sources(tst_compositor
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/idle-inhibit-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/ivi-application.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/viewporter.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/xdg-output-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/xdg-shell.xml
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_compositor CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
