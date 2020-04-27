# Generated from primaryselectionv1.pro.

#####################################################################
## tst_primaryselectionv1 Test:
#####################################################################

qt_add_test(tst_primaryselectionv1
    SOURCES
        ../shared/corecompositor.cpp ../shared/corecompositor.h
        ../shared/coreprotocol.cpp ../shared/coreprotocol.h
        ../shared/datadevice.cpp ../shared/datadevice.h
        ../shared/mockcompositor.cpp ../shared/mockcompositor.h
        ../shared/textinput.cpp ../shared/textinput.h
        ../shared/xdgoutputv1.cpp ../shared/xdgoutputv1.h
        ../shared/xdgshell.cpp ../shared/xdgshell.h
        tst_primaryselectionv1.cpp
    INCLUDE_DIRECTORIES
        ../shared
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Server
        Threads::Threads # special case
)

qt6_generate_wayland_protocol_server_sources(tst_primaryselectionv1
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/3rdparty/protocol/wp-primary-selection-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/../../../../src/3rdparty/protocol/text-input-unstable-v2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/../../../../src/3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/../../../../src/3rdparty/protocol/xdg-output-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/../../../../src/3rdparty/protocol/xdg-shell.xml
)
