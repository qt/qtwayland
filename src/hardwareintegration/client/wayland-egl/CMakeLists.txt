# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from wayland-egl.pro.

#####################################################################
## WaylandEglClientHwIntegrationPrivate Module:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_module(WaylandEglClientHwIntegrationPrivate
    CONFIG_MODULE_NAME wayland_egl_client_hw_integration
    INTERNAL_MODULE
    SOURCES
        qwaylandeglclientbufferintegration.cpp qwaylandeglclientbufferintegration_p.h
        qwaylandeglinclude_p.h
        qwaylandeglwindow.cpp qwaylandeglwindow_p.h
        qwaylandglcontext.cpp qwaylandglcontext_p.h
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC_LIBRARIES
        ${CMAKE_DL_LIBS}
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::OpenGLPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
        Wayland::Egl
)

#### Keys ignored in scope 1:.:.:wayland-egl.pro:<TRUE>:
# MODULE = "wayland_egl_client_hw_integration"
