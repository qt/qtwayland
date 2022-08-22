# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from src.pro.
# special case begin
qt_find_package(WaylandScanner PROVIDED_TARGETS Wayland::Scanner)
qt_find_package(Wayland 1.15 PROVIDED_TARGETS Wayland::Client Wayland::Server)

if (NOT WaylandScanner_FOUND OR NOT Wayland_FOUND)
    message(WARNING "QtWayland is missing required dependencies, nothing will be built. \
Although this could be considered an error, the configuration will still pass as coin (Qt's \
continuous integration system) will fail the build if configure fails, but will still try to \
configure the module on targets that are missing dependencies.")
    return()
endif()

# See global/README for a description of the following module.
qt_internal_add_module(WaylandGlobalPrivate INTERNAL_MODULE HEADER_MODULE)
add_subdirectory(qtwaylandscanner)

# special case begin
# TODO: Ideally these macros would be part of the qtwaylandscanner tool, and not the compositor/client
include(client/Qt6WaylandClientMacros.cmake)
include(compositor/Qt6WaylandCompositorMacros.cmake)
#special case end

if (QT_FEATURE_wayland_client)
    add_subdirectory(client)
endif()

if (QT_FEATURE_wayland_server)
    add_subdirectory(compositor)
endif()

if (QT_FEATURE_wayland_server OR QT_FEATURE_wayland_client)
    add_subdirectory(hardwareintegration)
    add_subdirectory(plugins)
    add_subdirectory(imports)
endif()
# special case end
