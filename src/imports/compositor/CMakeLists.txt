# Generated from compositor.pro.

#####################################################################
## qwaylandcompositorplugin Plugin:
#####################################################################

qt_add_qml_module(qwaylandcompositorplugin
    URI "QtWayland.Compositor"
    VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    CLASSNAME QWaylandCompositorPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        qwaylandmousetracker.cpp qwaylandmousetracker_p.h
        qwaylandquickcompositorplugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::WaylandCompositorPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::WaylandCompositor
)
#### Ignored generated resource: ${CMAKE_CURRENT_BINARY_DIR}/compositor.qrc
#### Keys ignored in scope 1:.:.:compositor.pro:<TRUE>:
# COMPOSITOR_QML_FILES = "WaylandOutputWindow.qml" "WaylandCursorItem.qml"
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# QMAKE_QMLPLUGINDUMP_FLAGS = "-defaultplatform"
# RESOURCE_CONTENT = "<RCC>" "<qresource prefix="/QtWayland/Compositor">" "</qresource>" "</RCC>"
# TARGETPATH = "QtWayland/Compositor"

## Scopes:
#####################################################################

#### Keys ignored in scope 3:.:.:compositor.pro:(CMAKE_BUILD_TYPE STREQUAL Debug):
# QML_FILES = "$$COMPOSITOR_QML_FILES"
