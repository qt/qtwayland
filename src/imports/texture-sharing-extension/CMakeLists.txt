# Generated from texture-sharing-extension.pro.

#####################################################################
## qwaylandtexturesharingextension Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandtexturesharingextension
    URI "QtWayland.Compositor.TextureSharingExtension"
    VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    CLASSNAME QWaylandTextureSharingExtensionPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        plugin.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::QuickPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

#### Keys ignored in scope 1:.:.:texture-sharing-extension.pro:<TRUE>:
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWayland/Compositor/TextureSharingExtension"
