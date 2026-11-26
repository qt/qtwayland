# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from src.pro.
# special case begin
if (TARGET Wayland::Scanner)
    qt_internal_disable_find_package_global_promotion(Wayland::Scanner)
endif()
qt_find_package(WaylandScanner PROVIDED_TARGETS Wayland::Scanner)

set(QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES "QT_COMMERCIAL_OR_GPL3")

set(wayland_libs
    Wayland::Client
    Wayland::Server
    Wayland::Cursor
    Wayland::Egl
)

foreach(lib IN LISTS wayland_libs)
    if(TARGET ${lib})
        qt_internal_disable_find_package_global_promotion(${lib})
    endif()
endforeach()

qt_find_package(Wayland 1.15 PROVIDED_TARGETS ${wayland_libs})

if (NOT WaylandScanner_FOUND OR NOT Wayland_FOUND)
    message(WARNING "QtWayland is missing required dependencies, nothing will be built. \
Although this could be considered an error, the configuration will still pass as coin (Qt's \
continuous integration system) will fail the build if configure fails, but will still try to \
configure the module on targets that are missing dependencies.")
    return()
endif()

if (QT_FEATURE_wayland_server)
    add_subdirectory(compositor)
endif()

if (QT_FEATURE_wayland_client)
    add_subdirectory(client)
endif()

if (QT_FEATURE_wayland_server OR QT_FEATURE_wayland_client)
    add_subdirectory(hardwareintegration)
    add_subdirectory(plugins)
    add_subdirectory(imports)
endif()
# special case end
