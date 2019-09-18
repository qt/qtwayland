# Generated from libhybris-egl-server.pro.

#####################################################################
## libhybris-egl-server Plugin:
#####################################################################

qt_add_plugin(libhybris-egl-server
    TYPE wayland-graphics-integration-client
    CLASS_NAME LibHybrisEglServerBufferPlugin
    SOURCES
        ../../../../hardwareintegration/client/libhybris-egl-server/libhybriseglserverbufferintegration.cpp ../../../../hardwareintegration/client/libhybris-egl-server/libhybriseglserverbufferintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/client/libhybris-egl-server
    LIBRARIES
        Qt::WaylandClientPrivate
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::WaylandClient
        Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(libhybris-egl-server
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/client/libhybris-egl-server/../../../extensions/libhybris-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:libhybris-egl-server.pro:<TRUE>:
# OTHER_FILES = "libhybris-egl-server.json"
