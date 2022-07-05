# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if(TARGET Qt::WaylandClient)
    add_subdirectory(qmlclient)
    add_subdirectory(subsurface)
    add_subdirectory(texture-sharing/cpp-client)
    add_subdirectory(texture-sharing-2)
endif()

if(TARGET Qt::WaylandCompositor)
    #add_subdirectory(wip-cpp-compositor)
    add_subdirectory(scaling-compositor)
endif()
