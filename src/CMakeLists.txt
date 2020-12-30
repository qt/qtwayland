# Generated from src.pro.
# special case begin
qt_find_package(WaylandScanner PROVIDED_TARGETS Wayland::Scanner)
qt_find_package(Wayland 1.15 PROVIDED_TARGETS Wayland::Client Wayland::Server)

if (NOT WaylandScanner_FOUND OR NOT Wayland_FOUND)
    message(WARNING "QtWayland is missing required dependencies, nothing will be built. \
Although this could be considered an error, the configuration will still pass as coin (Qt's \
continuous integration system) will fail the build if configure fails, but will still try to \
configure the module on targets that are missing dependencies.")
    return()
endif()

# We conditionally add_subdirectory(client) based on a feature that is defined
# in client/configure.cmake, so we need this hack to make sure the feature is
# defined on this level as well.
# TODO: when we remove configure.json support and stop generating
# configure.cmake, we should move the features up one level and deduplicate the
# features shared with the compositor.
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qtwayland-client-config.h"
    PRIVATE_FILE "qtwayland-client-config_p.h"
)
include("${CMAKE_CURRENT_SOURCE_DIR}/client/configure.cmake")
qt_feature_module_end(NO_MODULE)

# Similar hack as above, but for the compositor
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qtwayland-compositor-config.h"
    PRIVATE_FILE "qtwayland-compositor-config_p.h"
)
include("${CMAKE_CURRENT_SOURCE_DIR}/compositor/configure.cmake")
qt_feature_module_end(NO_MODULE)
# special case end

add_subdirectory(qtwaylandscanner)

# special case begin
# TODO: Ideally these macros would be part of the qtwaylandscanner tool, and not the compositor/client
include(client/Qt6WaylandClientMacros.cmake)
include(compositor/Qt6WaylandCompositorMacros.cmake)
#special case end

if (QT_FEATURE_wayland_client)
    add_subdirectory(client)
endif()

if (QT_FEATURE_wayland_server)
    add_subdirectory(compositor)
endif()

if (QT_FEATURE_wayland_server OR QT_FEATURE_wayland_client)
    add_subdirectory(hardwareintegration)
    add_subdirectory(plugins)
    add_subdirectory(imports)
endif()
# special case end
