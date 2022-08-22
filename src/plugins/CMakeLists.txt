# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from plugins.pro.

add_subdirectory(hardwareintegration)
if(TARGET Qt::WaylandClient)
    add_subdirectory(platforms)
    add_subdirectory(decorations)
    add_subdirectory(shellintegration)
endif()
