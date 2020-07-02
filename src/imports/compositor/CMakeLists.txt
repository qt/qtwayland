# Generated from compositor.pro.

#####################################################################
## qwaylandcompositorplugin Plugin:
#####################################################################

qt_add_qml_module(qwaylandcompositorplugin
    URI "QtWayland.Compositor"
    VERSION "${CMAKE_PROJECT_VERSION}"
    CLASSNAME QWaylandCompositorPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        qwaylandmousetracker.cpp qwaylandmousetracker_p.h
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

# Resources:
set(compositor_resource_files
    "WaylandCursorItem.qml"
    "WaylandOutputWindow.qml"
)

qt_add_resource(qwaylandcompositorplugin "compositor"
    PREFIX
        "/QtWaylandCompositor"
    FILES
        ${compositor_resource_files}
)


#### Keys ignored in scope 1:.:.:compositor.pro:<TRUE>:
# CXX_MODULE = "qml"
# QMAKE_QMLPLUGINDUMP_FLAGS = "-defaultplatform"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtWayland/Compositor"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:compositor.pro:(CMAKE_BUILD_TYPE STREQUAL Debug):
# QML_FILES = "WaylandOutputWindow.qml" "WaylandCursorItem.qml"
