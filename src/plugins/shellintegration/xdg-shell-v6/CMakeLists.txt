# Generated from xdg-shell-v6.pro.

#####################################################################
## QWaylandXdgShellV6IntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandXdgShellV6IntegrationPlugin
    OUTPUT_NAME xdg-shell-v6
    TYPE wayland-shell-integration
    SOURCES
        main.cpp
        qwaylandxdgshellv6.cpp qwaylandxdgshellv6_p.h
        qwaylandxdgshellv6integration.cpp qwaylandxdgshellv6integration_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(QWaylandXdgShellV6IntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-shell-unstable-v6.xml
)

#### Keys ignored in scope 1:.:.:xdg-shell-v6.pro:<TRUE>:
# OTHER_FILES = "xdg-shell-v6.json"

## Scopes:
#####################################################################

qt_internal_extend_target(QWaylandXdgShellV6IntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
