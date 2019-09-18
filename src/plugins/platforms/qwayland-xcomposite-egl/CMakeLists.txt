# Generated from qwayland-xcomposite-egl.pro.

#####################################################################
## qwayland-xcomposite-egl Plugin:
#####################################################################

qt_add_plugin(qwayland-xcomposite-egl
    TYPE platforms
    CLASS_NAME QWaylandXCompositeEglPlatformIntegrationPlugin
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
        Qt::EglSupportPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        EGL::EGL
        PkgConfig::XComposite
        Qt::Core
        Qt::EglSupport
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
        X11::X11
)

qt6_generate_wayland_protocol_client_sources(qwayland-xcomposite-egl
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../hardwareintegration/client/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:qwayland-xcomposite-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-xcomposite-egl.json"
