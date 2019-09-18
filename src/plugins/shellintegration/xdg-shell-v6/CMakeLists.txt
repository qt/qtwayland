# Generated from xdg-shell-v6.pro.

#####################################################################
## xdg-shell-v6 Plugin:
#####################################################################

qt_add_plugin(xdg-shell-v6
    TYPE wayland-shell-integration
    CLASS_NAME QWaylandXdgShellV6IntegrationPlugin
    SOURCES
        main.cpp
        qwaylandxdgshellv6.cpp qwaylandxdgshellv6_p.h
        qwaylandxdgshellv6integration.cpp qwaylandxdgshellv6integration_p.h
    LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(xdg-shell-v6
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-shell-unstable-v6.xml
)

#### Keys ignored in scope 1:.:.:xdg-shell-v6.pro:<TRUE>:
# OTHER_FILES = "xdg-shell-v6.json"

## Scopes:
#####################################################################

qt_extend_target(xdg-shell-v6 CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
