# Generated from wl-shell.pro.

#####################################################################
## wl-shell Plugin:
#####################################################################

qt_add_plugin(wl-shell
    TYPE wayland-shell-integration
    CLASS_NAME QWaylandWlShellIntegrationPlugin
    SOURCES
        main.cpp
        qwaylandwlshellintegration.cpp qwaylandwlshellintegration_p.h
        qwaylandwlshellsurface.cpp qwaylandwlshellsurface_p.h
    LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(wl-shell
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/wayland.xml
)

#### Keys ignored in scope 1:.:.:wl-shell.pro:<TRUE>:
# OTHER_FILES = "wl-shell.json"

## Scopes:
#####################################################################

qt_extend_target(wl-shell CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
