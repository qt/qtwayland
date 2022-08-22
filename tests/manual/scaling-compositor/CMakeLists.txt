# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from scaling-compositor.pro.

#####################################################################
## scaling-compositor Binary:
#####################################################################

qt_internal_add_manual_test(scaling-compositor
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(scaling-compositor "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:scaling-compositor.pro:<TRUE>:
# TEMPLATE = "app"
