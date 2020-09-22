# Generated from compositor.pro.

#####################################################################
## qwaylandcompositorplugin Plugin:
#####################################################################

qt_internal_add_qml_module(qwaylandcompositorplugin
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
# OTHER_FILES = "qmldir" "$$QML_FILES"
# QML_FILES = "$$PWD/WaylandOutputWindow.qml" "$$PWD/WaylandCursorItem.qml"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# QMl_IMPORT_NAME = "QtWayland.Compositor"
# TARGETPATH = "QtWayland/Compositor"

set(qml_files
    "WaylandOutputWindow.qml"
    "WaylandCursorItem.qml"
)
set_source_files_properties(WaylandOutputWindow.qml PROPERTIES
    QT_QML_SOURCE_VERSION "1.0"
)
set_source_files_properties(WaylandCursorItem.qml PROPERTIES
    QT_QML_SOURCE_VERSION "1.0"
)

qt6_target_qml_files(qwaylandcompositorplugin
    FILES
        ${qml_files}
)
