# Generated from xcomposite-egl.pro.

#####################################################################
## qt-wayland-compositor-xcomposite-egl Plugin:
#####################################################################

qt_add_plugin(qt-wayland-compositor-xcomposite-egl
    TYPE wayland-graphics-integration-server
    CLASS_NAME QWaylandXCompositeEglClientBufferIntegrationPlugin
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
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositorPrivate
        Wayland::Server
        X11::X11
    PUBLIC_LIBRARIES
        PkgConfig::XComposite
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

qt6_generate_wayland_protocol_server_sources(qt-wayland-compositor-xcomposite-egl
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:xcomposite-egl.pro:<TRUE>:
# OTHER_FILES = "xcomposite-egl.json"
