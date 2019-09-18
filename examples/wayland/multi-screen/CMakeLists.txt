# Generated from multi-screen.pro.

cmake_minimum_required(VERSION 3.14)
project(multi-screen LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/wayland/multi-screen")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

add_qt_gui_executable(multi-screen
    main.cpp
)
target_link_libraries(multi-screen PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(multi-screen_resource_files
    "qml/Chrome.qml"
    "qml/Screen.qml"
    "qml/main.qml"
)

qt6_add_resources(multi-screen "multi-screen"
    PREFIX
        "/"
    FILES
        ${multi-screen_resource_files}
)

install(TARGETS multi-screen
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
