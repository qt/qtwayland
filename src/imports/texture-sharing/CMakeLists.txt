# Generated from texture-sharing.pro.

#####################################################################
## qwaylandtexturesharing Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandtexturesharing
    URI "QtWayland.Client.TextureSharing"
    VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    CLASSNAME QWaylandTextureSharingPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        plugin.cpp
        sharedtextureprovider.cpp sharedtextureprovider.h
        texturesharingextension.cpp texturesharingextension.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::OpenGL
        Qt::Qml
        Qt::QuickPrivate
        Qt::WaylandClient
        Qt::WaylandClientPrivate
)

qt6_generate_wayland_protocol_client_sources(qwaylandtexturesharing
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../extensions/qt-texture-sharing-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:texture-sharing.pro:<TRUE>:
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWayland/Client/TextureSharing"
