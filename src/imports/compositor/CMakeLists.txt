# Generated from compositor.pro.

#####################################################################
## qwaylandcompositorplugin Plugin:
#####################################################################

qt_add_qml_module(qwaylandcompositorplugin
    URI "QtWayland.Compositor"
    VERSION "${CMAKE_PROJECT_VERSION}"
    CLASSNAME QWaylandCompositorPlugin
    SOURCES
        qwaylandquickcompositorplugin.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
)

#### Keys ignored in scope 1:.:.:compositor.pro:<TRUE>:
# CXX_MODULE = "qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# QMl_IMPORT_NAME = "QtWayland.Compositor"
# TARGETPATH = "QtWayland/Compositor"
