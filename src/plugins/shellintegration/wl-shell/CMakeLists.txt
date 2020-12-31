# Generated from wl-shell.pro.

#####################################################################
## WlShellIntegration Module:
#####################################################################

qt_internal_add_module(WlShellIntegration
    CONFIG_MODULE_NAME wl_shell_integration
    INTERNAL_MODULE
    SOURCES
        qwaylandwlshellintegration.cpp qwaylandwlshellintegration_p.h
        qwaylandwlshellsurface.cpp qwaylandwlshellsurface_p.h
    PUBLIC_LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(WlShellIntegration
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/wayland.xml
)

#### Keys ignored in scope 2:.:.:wl-shell-integration.pro:<TRUE>:
# MODULE = "wl_shell_integration"

## Scopes:
#####################################################################

qt_internal_extend_target(WlShellIntegration CONDITION QT_FEATURE_xkbcommon
    LIBRARIES
        XKB::XKB
)
#####################################################################
## QWaylandWlShellIntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWaylandWlShellIntegrationPlugin
    OUTPUT_NAME wl-shell-plugin
    TYPE wayland-shell-integration
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
        Qt::WlShellIntegrationPrivate
)

#### Keys ignored in scope 4:.:.:wl-shell-plugin.pro:<TRUE>:
# OTHER_FILES = "wl-shell.json"
