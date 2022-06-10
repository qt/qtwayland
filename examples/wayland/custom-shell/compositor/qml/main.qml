// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtQuick.Layouts
import QtQuick.Controls

import io.qt.examples 1.0

WaylandCompositor {
    id: comp

    ListModel { id: shellSurfaces }


    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Image {
                    fillMode: Image.Tile
                    source: "qrc:/images/background.png"
                    smooth: false
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 3
                        Repeater {
                            model: shellSurfaces
                            ShellSurfaceItem {
                                id: chrome
                                shellSurface: modelData
                                visible: !shellSurface.minimized
                                onSurfaceDestroyed: shellSurfaces.remove(index)
                            }
                        }
                    }
                }
                Row {
                    id: taskbar
                    Layout.fillWidth: true
                    Repeater {
                        model: shellSurfaces
                        Button {
                            id: minimizeButton
                            checkable: true
                            text: modelData.windowTitle
                            onCheckedChanged: modelData.minimized = checked
                            checked: modelData.minimized
                        }
                    }
                }
            }
        }
    }

    //! [ExampleShell]
    ExampleShell {
        id: shell
        onShellSurfaceCreated: (shellSurface) => {
            shellSurfaces.append({shellSurface: shellSurface});
        }
    }
    //! [ExampleShell]
}
