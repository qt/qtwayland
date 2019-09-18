# Generated from vsp2.pro.

#####################################################################
## qt-wayland-compositor-vsp2 Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-vsp2
    TYPE wayland-hardware-layer-integration
    CLASS_NAME Vsp2HardwareLayerIntegrationPlugin
    SOURCES
        ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2/vsp2hardwarelayerintegration.cpp ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2/vsp2hardwarelayerintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../../hardwareintegration/compositor/hardwarelayer/vsp2
    LIBRARIES
        PkgConfig::Waylandkms
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:vsp2.pro:<TRUE>:
# OTHER_FILES = "vsp2.json"
