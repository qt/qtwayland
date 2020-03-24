# Generated from bradient.pro.

#####################################################################
## QWaylandBradientDecorationPlugin Plugin:
#####################################################################

qt_add_plugin(QWaylandBradientDecorationPlugin
    OUTPUT_NAME bradient
    TYPE wayland-decoration-client
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
)

#### Keys ignored in scope 1:.:.:bradient.pro:<TRUE>:
# OTHER_FILES = "bradient.json"
