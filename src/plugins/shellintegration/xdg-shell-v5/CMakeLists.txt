# Generated from xdg-shell-v5.pro.

#####################################################################
## QWaylandXdgShellV5IntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandXdgShellV5IntegrationPlugin
    OUTPUT_NAME xdg-shell-v5
    TYPE wayland-shell-integration
    SOURCES
        main.cpp
        pregenerated/3rdparty/qwayland-xdg-shell-unstable-v5.cpp pregenerated/3rdparty/qwayland-xdg-shell-unstable-v5_p.h
        pregenerated/3rdparty/wayland-xdg-shell-unstable-v5-client-protocol_p.h
        pregenerated/3rdparty/wayland-xdg-shell-unstable-v5-protocol.c
        qwaylandxdgpopupv5.cpp qwaylandxdgpopupv5_p.h
        qwaylandxdgshellv5.cpp qwaylandxdgshellv5_p.h
        qwaylandxdgshellv5integration.cpp qwaylandxdgshellv5integration_p.h
        qwaylandxdgsurfacev5.cpp qwaylandxdgsurfacev5_p.h
    INCLUDE_DIRECTORIES
        pregenerated/3rdparty
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

#### Keys ignored in scope 1:.:.:xdg-shell-v5.pro:<TRUE>:
# OTHER_FILES = "xdg-shell-v5.json"

## Scopes:
#####################################################################

qt_extend_target(QWaylandXdgShellV5IntegrationPlugin CONDITION QT_FEATURE_xkbcommon
    PUBLIC_LIBRARIES
        XKB::XKB
)
