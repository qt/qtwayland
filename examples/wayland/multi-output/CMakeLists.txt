# Generated from multi-output.pro.

cmake_minimum_required(VERSION 3.14)
project(multi-output LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/wayland/multi-output")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

add_qt_gui_executable(multi-output
    main.cpp
)
target_link_libraries(multi-output PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(multi-output_resource_files
    "images/background.jpg"
    "qml/GridScreen.qml"
    "qml/ShellChrome.qml"
    "qml/ShellScreen.qml"
    "qml/main.qml"
)

qt6_add_resources(multi-output "multi-output"
    PREFIX
        "/"
    FILES
        ${multi-output_resource_files}
)

install(TARGETS multi-output
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
