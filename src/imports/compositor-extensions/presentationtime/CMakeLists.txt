#####################################################################
## qwaylandcompositorpresentationtimeplugin Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandCompositorPresentationTime
    URI "QtWayland.Compositor.PresentationTime"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QWaylandCompositorPresentationTimePlugin
    PLUGIN_TARGET WaylandCompositorPresentationTime
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    SOURCES
        qwaylandcompositorpresentationtimeplugin.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandCompositorPrivate
)
