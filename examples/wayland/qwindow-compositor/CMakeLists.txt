# Generated from qwindow-compositor.pro.

cmake_minimum_required(VERSION 3.14)
project(qwindow-compositor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/qwindow-compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WaylandCompositor)

add_qt_gui_executable(qwindow-compositor
    compositor.cpp compositor.h
    main.cpp
    window.cpp window.h
)
target_include_directories(qwindow-compositor PUBLIC
    ../../include
)

target_link_libraries(qwindow-compositor PUBLIC
    # Remove: L
    #../../lib # special case
    Qt::Core
    Qt::Gui
    Qt::WaylandCompositor
)


# Resources:
set_source_files_properties("../pure-qml/images/background.jpg"
    PROPERTIES QT_RESOURCE_ALIAS "background.jpg"
)
set(qwindow-compositor_resource_files
    "../pure-qml/images/background.jpg"
)

qt6_add_resources(qwindow-compositor "qwindow-compositor"
    PREFIX
        "/"
    FILES
        ${qwindow-compositor_resource_files}
)

install(TARGETS qwindow-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
