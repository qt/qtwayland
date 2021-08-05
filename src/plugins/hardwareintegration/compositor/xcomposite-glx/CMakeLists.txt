# Generated from xcomposite-glx.pro.

#####################################################################
## QWaylandXCompositeGlxClientBufferIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandXCompositeGlxClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-xcomposite-glx
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/xcomposite-glx/xcompositeglxintegration.cpp ../../../../hardwareintegration/compositor/xcomposite-glx/xcompositeglxintegration.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xcompositebuffer.cpp ../../../../hardwareintegration/compositor/xcomposite_share/xcompositebuffer.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xcompositehandler.cpp ../../../../hardwareintegration/compositor/xcomposite_share/xcompositehandler.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xlibinclude.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/xcomposite-glx
        ../../../../hardwareintegration/compositor/xcomposite_share
    LIBRARIES
        PkgConfig::XComposite
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Server
        X11::X11
)

qt6_generate_wayland_protocol_server_sources(QWaylandXCompositeGlxClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:xcomposite-glx.pro:<TRUE>:
# OTHER_FILES = "xcomposite-glx.json"
