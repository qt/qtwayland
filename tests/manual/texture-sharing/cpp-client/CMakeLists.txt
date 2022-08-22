# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from cpp-client.pro.

#####################################################################
## cpp-client Binary:
#####################################################################

qt_internal_add_manual_test(cpp-client
    GUI
    SOURCES
        ../../../../src/imports/texture-sharing/texturesharingextension.cpp
        ../../../../src/imports/texture-sharing/texturesharingextension_p.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/imports/texture-sharing
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::OpenGL
        Qt::WaylandClientPrivate
)

qt6_generate_wayland_protocol_client_sources(cpp-client
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/extensions/qt-texture-sharing-unstable-v1.xml
)

#### Keys ignored in scope 1:.:.:cpp-client.pro:<TRUE>:
# INSTALLS = "target"
# target.path = "$$[QT_INSTALL_EXAMPLES]/wayland/texture-sharing/cpp-client"
