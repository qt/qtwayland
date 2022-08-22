# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from subsurface.pro.

#####################################################################
## subsurface Binary:
#####################################################################

qt_internal_add_manual_test(subsurface
    GUI
    SOURCES
        main.cpp
        shmwindow.cpp shmwindow.h
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
        Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "child.qml"
    "main.qml"
)

qt_internal_add_resource(subsurface "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:subsurface.pro:<TRUE>:
# TEMPLATE = "app"
