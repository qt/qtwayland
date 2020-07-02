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
#### Ignored generated resource: ${CMAKE_CURRENT_BINARY_DIR}/compositor.qrc
#### Keys ignored in scope 1:.:.:compositor.pro:<TRUE>:
# COMPOSITOR_QML_FILES = "WaylandOutputWindow.qml" "WaylandCursorItem.qml"
# CXX_MODULE = "qml"
# QMAKE_QMLPLUGINDUMP_FLAGS = "-defaultplatform"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# RESOURCE_CONTENT = "<RCC>" "<qresource prefix="/QtWayland/Compositor">" "</qresource>" "</RCC>"
# TARGETPATH = "QtWayland/Compositor"

## Scopes:
#####################################################################

#### Keys ignored in scope 3:.:.:compositor.pro:(CMAKE_BUILD_TYPE STREQUAL Debug):
# QML_FILES = "$$COMPOSITOR_QML_FILES"
