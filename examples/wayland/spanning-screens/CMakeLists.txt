# Generated from spanning-screens.pro.

cmake_minimum_required(VERSION 3.14)
project(spanning-screens LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/spanning-screens")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(spanning-screens
    main.cpp
)
set_target_properties(spanning-screens PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(spanning-screens PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(spanning-screens_resource_files
    "main.qml"
)

qt6_add_resources(spanning-screens "spanning-screens"
    PREFIX
        "/"
    FILES
        ${spanning-screens_resource_files}
)

install(TARGETS spanning-screens
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
