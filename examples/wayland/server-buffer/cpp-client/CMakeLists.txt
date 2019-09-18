# Generated from cpp-client.pro.

cmake_minimum_required(VERSION 3.14)
project(server-buffer-cpp-client LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/wayland/server-buffer/cpp-client")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WaylandClient)

add_qt_gui_executable(server-buffer-cpp-client
    main.cpp
    sharebufferextension.cpp sharebufferextension.h
)

qt6_generate_wayland_protocol_client_sources(server-buffer-cpp-client
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../share-buffer.xml
)
target_link_libraries(server-buffer-cpp-client PRIVATE
    Qt::GuiPrivate
    Qt::WaylandClientPrivate
)

target_link_libraries(server-buffer-cpp-client PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WaylandClient
)

install(TARGETS server-buffer-cpp-client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
