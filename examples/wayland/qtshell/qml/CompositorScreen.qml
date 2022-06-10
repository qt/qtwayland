// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtWayland.Compositor

WaylandOutput {
    id: output

    property bool isNestedCompositor: Qt.platform.pluginName.startsWith("wayland") || Qt.platform.pluginName === "xcb"

    //! [handleShellSurface]
    property ListModel shellSurfaces: ListModel {}
    function handleShellSurface(shellSurface) {
        shellSurfaces.append({shellSurface: shellSurface});
    }
    //! [handleShellSurface]

    // During development, it can be useful to start the compositor inside X11 or
    // another Wayland compositor. In such cases, set sizeFollowsWindow to true to
    // enable resizing of the compositor window to be forwarded to the Wayland clients
    // as the output (screen) changing resolution. Consider setting it to false if you
    // are running the compositor using eglfs, linuxfb or similar QPA backends.
    sizeFollowsWindow: output.isNestedCompositor

    window: Window {
        width: 1920
        height: 1080
        visible: true

        WaylandMouseTracker {
            id: mouseTracker

            anchors.fill: parent

            // Set this to false to disable the outer mouse cursor when running nested
            // compositors. Otherwise you would see two mouse cursors, one for each compositor.
            windowSystemCursorEnabled: output.isNestedCompositor

            Image {
                id: background

                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.jpg"
                smooth: true

                //! [repeater]
                Repeater {
                    id: chromeRepeater
                    model: output.shellSurfaces
                    // Chrome displays a shell surface on the screen (See Chrome.qml)
                    Chrome {
                        shellSurface: modelData
                        onClientDestroyed:
                        {
                            output.shellSurfaces.remove(index)
                        }
                    }
                }
                //! [repeater]
            }

            Rectangle {
                anchors.fill: taskbar
                color: "lavenderblush"
            }

            //! [taskbar]
            Row {
                id: taskbar
                height: 40
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                Repeater {
                    anchors.fill: parent
                    model: output.shellSurfaces

                    ToolButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.windowTitle
                        onClicked: {
                            var item = chromeRepeater.itemAt(index)
                            if ((item.windowState & Qt.WindowMinimized) != 0)
                                item.toggleMinimized()
                            chromeRepeater.itemAt(index).activate()
                        }
                    }
                }
            }
            //! [taskbar]

            //! [usableArea]
            Item {
                id: usableArea
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: taskbar.top
            }
            //! [usableArea]
        }
    }
}
