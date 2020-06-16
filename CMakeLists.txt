# Generated from qtwayland.pro.

cmake_minimum_required(VERSION 3.15.0)

include(.cmake.conf)
project(QtWayland # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Wayland Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
# TODO: Fix warnings and remove this
set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)

# TODO: Remove this after qtbase revert of
# dd7e40b1086020f6a054957f4972720cd684.
# Needed for correct dependency tracking of Wayland::Client
# and Wayland::Server in the separate qt_find_package() calls
# so that standalone tests configure successfully.
set(QT_FIND_PACKAGE_DISABLE_DEBUG_BEHAVIOR TRUE)

# Find BuildInternals first by itself because it defines QNX
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    BuildInternals
)

if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
    return()
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    OpenGL
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS
    EglSupport
    GlxSupport
    LinuxAccessibilitySupport
    Quick
    ServiceSupport
    XkbCommonSupport
    ThemeSupport
)
# special case end

# special case begin
# Moved above.
#if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
#    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
#    return()
#endif()
# special case end
if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
