# Generated from qwayland-xcomposite-glx.pro.

#####################################################################
## QWaylandXCompositeGlxPlatformIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandXCompositeGlxPlatformIntegrationPlugin
    OUTPUT_NAME qwayland-xcomposite-glx
    PLUGIN_TYPE platforms
    SOURCES
        ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxcontext.cpp ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxcontext.h
        ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxintegration.cpp ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxintegration.h
        ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxwindow.cpp ../../../hardwareintegration/client/xcomposite-glx/qwaylandxcompositeglxwindow.h
        ../../../hardwareintegration/client/xcomposite_share/qwaylandxcompositebuffer.cpp ../../../hardwareintegration/client/xcomposite_share/qwaylandxcompositebuffer.h
        main.cpp
        qwaylandxcompositeglxplatformintegration.h
    INCLUDE_DIRECTORIES
        ../../../hardwareintegration/client/xcomposite-glx
        ../../../hardwareintegration/client/xcomposite_share
    LIBRARIES
        ${CMAKE_DL_LIBS}
        PkgConfig::XComposite
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
        X11::X11
)

qt6_generate_wayland_protocol_client_sources(QWaylandXCompositeGlxPlatformIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../hardwareintegration/client/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:qwayland-xcomposite-glx.pro:<TRUE>:
# OTHER_FILES = "qwayland-xcomposite-glx.json"
