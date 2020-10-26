# Generated from ivi-compositor.pro.

cmake_minimum_required(VERSION 3.14)
project(ivi-compositor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/ivi-compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(ivi-compositor
    main.cpp
)
set_target_properties(ivi-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(ivi-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(ivi-compositor_resource_files
    "main.qml"
)

qt6_add_resources(ivi-compositor "ivi-compositor"
    PREFIX
        "/"
    FILES
        ${ivi-compositor_resource_files}
)

install(TARGETS ivi-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
