# Generated from xdgshell.pro.

#####################################################################
## qwaylandcompositorxdgshellplugin Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandcompositorxdgshellplugin
    URI "QtWayland.Compositor.XdgShell"
    VERSION "${CMAKE_PROJECT_VERSION}"
    CLASSNAME QWaylandCompositorXdgShellPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        qwaylandcompositorxdgshellplugin.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositor
)

#### Keys ignored in scope 1:.:.:xdgshell.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor/XdgShell"
