# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from hardwareintegration.pro.

# When doing unity build, we get symbol collisions with macros defined
# in X11.h, and Xlib.h
set(CMAKE_UNITY_BUILD OFF)

if(TARGET Qt::WaylandClient)
    add_subdirectory(client)
endif()
if(TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()

set(CMAKE_UNITY_BUILD ${QT_UNITY_BUILD})
