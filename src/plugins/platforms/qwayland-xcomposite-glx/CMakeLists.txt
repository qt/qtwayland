# Generated from qwayland-xcomposite-glx.pro.

#####################################################################
## qwayland-xcomposite-glx Plugin:
#####################################################################

qt_add_plugin(qwayland-xcomposite-glx
    TYPE platforms
    CLASS_NAME QWaylandXCompositeGlxPlatformIntegrationPlugin
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
        Qt::GlxSupportPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        ${CMAKE_DL_LIBS}
        PkgConfig::XComposite
        Qt::Core
        Qt::GlxSupport
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
        X11::X11
)

qt6_generate_wayland_protocol_client_sources(qwayland-xcomposite-glx
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../hardwareintegration/client/xcomposite_share/../../../extensions/xcomposite.xml
)

#### Keys ignored in scope 1:.:.:qwayland-xcomposite-glx.pro:<TRUE>:
# OTHER_FILES = "qwayland-xcomposite-glx.json"
