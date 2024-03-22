# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtwayland.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtWayland # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Wayland Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    BuildInternals
)
qt_internal_project_setup()

if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
    return()
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
)

find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
    DBus
    OpenGL
    Quick
    Svg
)

# special case begin
# Moved above.
#if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
#    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
#    return()
#endif()
# special case end
qt_build_repo()
