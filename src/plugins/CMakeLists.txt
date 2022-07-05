# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from plugins.pro.

add_subdirectory(hardwareintegration)
if(TARGET Qt::WaylandClient)
    add_subdirectory(platforms)
    add_subdirectory(decorations)
    add_subdirectory(shellintegration)
endif()
