# Generated from qwayland-xcomposite-egl.pro.

#####################################################################
## QWaylandXCompositeEglPlatformIntegrationPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandXCompositeEglPlatformIntegrationPlugin
    OUTPUT_NAME qwayland-xcomposite-egl
    PLUGIN_TYPE platforms
    SOURCES
        ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglclientbufferintegration.cpp ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglclientbufferintegration.h
        ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglcontext.cpp ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglcontext.h
        ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglwindow.cpp ../../../hardwareintegration/client/xcomposite-egl/qwaylandxcompositeeglwindow.h
        ../../../hardwareintegration/client/xcomposite_share/qwaylandxcompositebuffer.cpp ../../../hardwareintegration/client/xcomposite_share/qwaylandxcompositebuffer.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../hardwareintegration/client/xcomposite-egl
        ../../../hardwareintegration/client/xcomposite_share
    LIBRARIES
        EGL::EGL
        PkgConfig::XComposite
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
        X11::X11
)

qt6_generate_wayland_protocol_client_sources(QWaylandXCompositeEglPlatformIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../hardwareintegration/client/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:qwayland-xcomposite-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-xcomposite-egl.json"
