# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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

if(QT_FEATURE_opengl AND TARGET Qt::Quick AND TARGET Qt::WaylandClient)
    qt_internal_add_example(server-buffer)
endif()
