# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from auto.pro.

if(TARGET Qt::WaylandClient)
    add_subdirectory(client)
endif()
if(TARGET Qt::WaylandClient AND TARGET Qt::WaylandCompositor)
    #add_subdirectory(cmake) # special case
endif()
if(TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()
