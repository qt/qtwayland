# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from brcm-egl.pro.

#####################################################################
## QWaylandBrcmEglClientBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandBrcmEglClientBufferPlugin
    OUTPUT_NAME brcm-egl
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglintegration.cpp ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglintegration.h
        ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglwindow.cpp ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglwindow.h
        ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmglcontext.cpp ../../../../hardwareintegration/client/brcm-egl/qwaylandbrcmglcontext.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/brcm-egl
    PUBLIC_LIBRARIES
        ${CMAKE_DL_LIBS}
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandBrcmEglClientBufferPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/brcm-egl/../../../extensions/brcm.xml
)

#### Keys ignored in scope 1:.:.:brcm-egl.pro:<TRUE>:
# OTHER_FILES = "brcm-egl.json"
