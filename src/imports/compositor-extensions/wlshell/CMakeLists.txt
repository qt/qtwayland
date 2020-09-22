# Generated from wlshell.pro.

#####################################################################
## qwaylandcompositorwlshellplugin Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandcompositorwlshellplugin
    URI "QtWayland.Compositor.WlShell"
    VERSION "${CMAKE_PROJECT_VERSION}"
    CLASSNAME QWaylandCompositorWlShellPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        qwaylandcompositorwlshellplugin.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:wlshell.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/WlShell"
