# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from imports.pro.

if(TARGET Qt::Quick AND TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor-extensions)
endif()
if(QT_FEATURE_opengl AND TARGET Qt::Quick AND TARGET Qt::WaylandClient AND TARGET Qt::WaylandCompositor)
    add_subdirectory(texture-sharing)
    add_subdirectory(texture-sharing-extension)
endif()
