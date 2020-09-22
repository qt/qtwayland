# Generated from iviapplication.pro.

#####################################################################
## qwaylandcompositoriviapplicationplugin Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandcompositoriviapplicationplugin
    URI "QtWayland.Compositor.IviApplication"
    VERSION "${CMAKE_PROJECT_VERSION}"
    CLASSNAME QWaylandCompositorIviApplicationPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        qwaylandcompositoriviapplicationplugin.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:iviapplication.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/IviApplication"
