# Generated from qwayland-generic.pro.

#####################################################################
## qwayland-generic Plugin:
#####################################################################

qt_add_plugin(qwayland-generic
    TYPE platforms
    CLASS_NAME QWaylandIntegrationPlugin
    DEFAULT_IF ${QT_QPA_DEFAULT_PLATFORM} MATCHES wayland # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
)

#### Keys ignored in scope 1:.:.:qwayland-generic.pro:<TRUE>:
# OTHER_FILES = "qwayland-generic.json"
