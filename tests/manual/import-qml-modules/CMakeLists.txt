# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(import_wayland_qml_modules VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED
    COMPONENTS
        Quick
        WaylandCompositor
        WaylandCompositorIviapplication
        WaylandCompositorPresentationTime
        WaylandCompositorWLShell
        WaylandCompositorXdgShell
)

qt_standard_project_setup()

qt_add_qml_module(import_wayland_qml_modules
    URI Qml
    QML_FILES Main.qml
)
