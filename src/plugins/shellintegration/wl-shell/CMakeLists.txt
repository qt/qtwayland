# Generated from wl-shell.pro.

#####################################################################
## QWaylandWlShellIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandWlShellIntegrationPlugin
    OUTPUT_NAME wl-shell
    TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandwlshellintegration.cpp qwaylandwlshellintegration_p.h
        qwaylandwlshellsurface.cpp qwaylandwlshellsurface_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandWlShellIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/wayland.xml
)

#### Keys ignored in scope 1:.:.:wl-shell.pro:<TRUE>:
# OTHER_FILES = "wl-shell.json"

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandWlShellIntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
