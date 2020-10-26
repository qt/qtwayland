# Generated from server-side-decoration.pro.

cmake_minimum_required(VERSION 3.14)
project(server-side-decoration LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/server-side-decoration")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(server-side-decoration
    main.cpp
)
set_target_properties(server-side-decoration PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(server-side-decoration PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(server-side-decoration_resource_files
    "main.qml"
)

qt6_add_resources(server-side-decoration "server-side-decoration"
    PREFIX
        "/"
    FILES
        ${server-side-decoration_resource_files}
)

install(TARGETS server-side-decoration
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
