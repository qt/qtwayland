# Generated from fullscreen-shell-v1.pro.

#####################################################################
## fullscreen-shell-v1 Plugin:
#####################################################################

qt_add_plugin(fullscreen-shell-v1
    TYPE wayland-shell-integration
    CLASS_NAME QWaylandFullScreenShellV1IntegrationPlugin
    SOURCES
        main.cpp
        qwaylandfullscreenshellv1integration.cpp qwaylandfullscreenshellv1integration.h
        qwaylandfullscreenshellv1surface.cpp qwaylandfullscreenshellv1surface.h
    LIBRARIES
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(fullscreen-shell-v1
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/protocol/fullscreen-shell-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:fullscreen-shell-v1.pro:<TRUE>:
# OTHER_FILES = "fullscreen-shell-v1.json"
