# Generated from cpp-client.pro.

cmake_minimum_required(VERSION 3.14)
project(server-buffer-cpp-client) # special case

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/wayland/server-buffer/cpp-client")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WaylandClient)
find_package(Qt6 COMPONENTS OpenGL)

qt_add_executable(server-buffer-cpp-client
    main.cpp
    sharebufferextension.cpp sharebufferextension.h
)

qt6_generate_wayland_protocol_client_sources(server-buffer-cpp-client
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../share-buffer.xml
)
set_target_properties(server-buffer-cpp-client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(server-buffer-cpp-client PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::OpenGL
    Qt::WaylandClientPrivate
)

install(TARGETS server-buffer-cpp-client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
