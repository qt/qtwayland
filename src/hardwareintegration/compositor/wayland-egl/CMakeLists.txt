# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## WaylandEglCompositorHwIntegrationPrivate Module:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_module(WaylandEglCompositorHwIntegrationPrivate
    CONFIG_MODULE_NAME wayland_egl_compositor_hw_integration
    INTERNAL_MODULE
    SOURCES
        waylandeglclientbufferintegration.cpp waylandeglclientbufferintegration_p.h
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        EGL::EGL
        Wayland::Egl
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositorPrivate
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(WaylandEglCompositorHwIntegrationPrivate CONDITION NOT QT_FEATURE_egl_x11
    DEFINES
        QT_EGL_NO_X11
)
