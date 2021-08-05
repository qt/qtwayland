# Generated from xcomposite-egl.pro.

#####################################################################
## QWaylandXCompositeEglClientBufferIntegrationPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandXCompositeEglClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-xcomposite-egl
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/xcomposite-egl/xcompositeeglintegration.cpp ../../../../hardwareintegration/compositor/xcomposite-egl/xcompositeeglintegration.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xcompositebuffer.cpp ../../../../hardwareintegration/compositor/xcomposite_share/xcompositebuffer.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xcompositehandler.cpp ../../../../hardwareintegration/compositor/xcomposite_share/xcompositehandler.h
        ../../../../hardwareintegration/compositor/xcomposite_share/xlibinclude.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/xcomposite-egl
        ../../../../hardwareintegration/compositor/xcomposite_share
    LIBRARIES
        EGL::EGL
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

qt6_generate_wayland_protocol_server_sources(QWaylandXCompositeEglClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:xcomposite-egl.pro:<TRUE>:
# OTHER_FILES = "xcomposite-egl.json"
