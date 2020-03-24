# Generated from tests.pro.

qt_find_package(Threads REQUIRED) # special case
if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
    # special case begin
    qt_find_package(Qt6 ${PROJECT_VERSION} OPTIONAL_COMPONENTS WaylandCompositor WaylandClient QtWaylandScannerTools)

    if (NOT Qt6QtWaylandScannerTools_FOUND)
        message(WARNING "QtWaylandTests is missing required components, nothing will be built. \
        Although this could be considered an error, the configuration will still pass as coin (Qt's \
        continuous integration system) will fail the build if configure fails, but will still try to \
        configure the module on targets that are missing dependencies.")
        return()
    endif()

    # TODO: Why do we need this? shouldn't find_package for QtWaylandScannerTools be enough?
    qt_find_package(WaylandScanner PROVIDED_TARGETS Wayland::Scanner)
    # special case end
endif()
qt_build_tests()
