# Generated from xdg-shell.pro.

#####################################################################
## xdg-shell Plugin:
#####################################################################

qt_add_plugin(xdg-shell
    TYPE wayland-shell-integration
    CLASS_NAME QWaylandXdgShellIntegrationPlugin
    SOURCES
        main.cpp
        qwaylandxdgdecorationv1.cpp qwaylandxdgdecorationv1_p.h
        qwaylandxdgshell.cpp qwaylandxdgshell_p.h
        qwaylandxdgshellintegration.cpp qwaylandxdgshellintegration_p.h
    LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(xdg-shell
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-decoration-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/xdg-shell.xml
)

#### Keys ignored in scope 1:.:.:xdg-shell.pro:<TRUE>:
# OTHER_FILES = "xdg-shell.json"

## Scopes:
#####################################################################

qt_extend_target(xdg-shell CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
