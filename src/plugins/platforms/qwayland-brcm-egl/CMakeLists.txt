# Generated from qwayland-brcm-egl.pro.

#####################################################################
## qwayland-brcm-egl Plugin:
#####################################################################

qt_add_plugin(qwayland-brcm-egl
    TYPE platforms
    CLASS_NAME QWaylandBrcmEglPlatformIntegrationPlugin
    SOURCES
        ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglintegration.cpp ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglintegration.h
        ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglwindow.cpp ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmeglwindow.h
        ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmglcontext.cpp ../../../hardwareintegration/client/brcm-egl/qwaylandbrcmglcontext.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../hardwareintegration/client/brcm-egl
    LIBRARIES
        Qt::EglSupportPrivate
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        ${CMAKE_DL_LIBS}
        EGL::EGL
        Qt::Core
        Qt::EglSupport
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(qwayland-brcm-egl
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../hardwareintegration/client/brcm-egl/../../../extensions/brcm.xml
)

#### Keys ignored in scope 1:.:.:qwayland-brcm-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-brcm-egl.json"
