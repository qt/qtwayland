# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from vsp2.pro.

#####################################################################
## Vsp2HardwareLayerIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(Vsp2HardwareLayerIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-vsp2
    PLUGIN_TYPE wayland-hardware-layer-integration
    SOURCES
        ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2/vsp2hardwarelayerintegration.cpp ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2/vsp2hardwarelayerintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2
    LIBRARIES
        PkgConfig::Waylandkms
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

#### Keys ignored in scope 1:.:.:vsp2.pro:<TRUE>:
# OTHER_FILES = "vsp2.json"
