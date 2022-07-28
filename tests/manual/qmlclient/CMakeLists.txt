# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qmlclient.pro.

#####################################################################
## qmlclient Binary:
#####################################################################

qt_internal_add_manual_test(qmlclient
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
        Qt::WaylandClient
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(qmlclient "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:qmlclient.pro:<TRUE>:
# TEMPLATE = "app"
