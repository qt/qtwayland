# Generated from compositor.pro.

cmake_minimum_required(VERSION 3.14)
project(custom-extension-compositor) # special case

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/custom-extension/compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS WaylandCompositor)

qt_add_executable(custom-extension-compositor
    customextension.cpp customextension.h
    main.cpp
)

qt6_generate_wayland_protocol_server_sources(custom-extension-compositor
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/custom.xml
)
set_target_properties(custom-extension-compositor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(custom-extension-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::WaylandCompositor
)


# Resources:
set(compositor_resource_files
    "images/background.png"
    "qml/CompositorScreen.qml"
    "qml/main.qml"
)

qt6_add_resources(custom-extension-compositor "compositor"
    PREFIX
        "/"
    FILES
        ${compositor_resource_files}
)

install(TARGETS custom-extension-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
