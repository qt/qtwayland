# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(qml-client
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Quick
)

# Resources:
set(qml-client_resource_files
    "main.qml"
)

qt_internal_add_resource(qml-client "qml-client"
    PREFIX
        "/"
    FILES
        ${qml-client_resource_files}
)
