# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from tests.pro.

# special case begin
# TODO: Prepare for removal, once Platform brings in Threads.
if(NOT TARGET Threads::Threads)
    qt_find_package(Threads REQUIRED)
endif()
# special case end

if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
    # special case begin
    qt_find_package(Qt6 ${PROJECT_VERSION} OPTIONAL_COMPONENTS WaylandCompositor WaylandClient)

    if (NOT Qt6WaylandScannerTools_FOUND)
        message(WARNING "QtWaylandTests is missing required components, nothing will be built. \
        Although this could be considered an error, the configuration will still pass as coin (Qt's \
        continuous integration system) will fail the build if configure fails, but will still try to \
        configure the module on targets that are missing dependencies.")
        return()
    endif()
    # special case end
endif()
qt_build_tests()
