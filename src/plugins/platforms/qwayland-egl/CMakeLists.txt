# Generated from qwayland-egl.pro.

#####################################################################
## QWaylandEglPlatformIntegrationPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandEglPlatformIntegrationPlugin
    OUTPUT_NAME qwayland-egl
    TYPE platforms
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Qt::WaylandEglClientHwIntegrationPrivate
)

#### Keys ignored in scope 1:.:.:qwayland-egl.pro:<TRUE>:
# OTHER_FILES = "qwayland-egl.json"
