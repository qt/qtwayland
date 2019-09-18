# Generated from bradient.pro.

#####################################################################
## bradient Plugin:
#####################################################################

qt_add_plugin(bradient
    TYPE wayland-decoration-client
    CLASS_NAME QWaylandBradientDecorationPlugin
    SOURCES
        main.cpp
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

#### Keys ignored in scope 1:.:.:bradient.pro:<TRUE>:
# OTHER_FILES = "bradient.json"
