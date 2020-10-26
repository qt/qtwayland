# Generated from hwlayer-compositor.pro.

cmake_minimum_required(VERSION 3.14)
project(hwlayer-compositor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/hwlayer-compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(hwlayer-compositor
    main.cpp
)
set_target_properties(hwlayer-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(hwlayer-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(hwlayer-compositor_resource_files
    "main.qml"
)

qt6_add_resources(hwlayer-compositor "hwlayer-compositor"
    PREFIX
        "/"
    FILES
        ${hwlayer-compositor_resource_files}
)

install(TARGETS hwlayer-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
