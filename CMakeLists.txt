# Generated from qtwayland.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtWayland # special case
    VERSION 6.0.0
    DESCRIPTION "Qt Wayland Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
# TODO: Fix warnings and remove this
set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    BuildInternals
    Core
    Gui
    EventDispatcherSupport
    FontDatabaseSupport
    ThemeSupport
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS
    EglSupport
    GlxSupport
    LinuxAccessibilitySupport
    Quick
    ServiceSupport
    VulkanSupport
    XkbCommonSupport
)
# special case end

if(NOT APPLE_OSX AND (ANDROID OR NOT LINUX))
    message(NOTICE "Skipping the build as the condition \"APPLE_OSX OR LINUX\" is not met.")
    return()
endif()
if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
