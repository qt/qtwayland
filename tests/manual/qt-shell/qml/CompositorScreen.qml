// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtWayland.Compositor

WaylandOutput {
    id: output

    property ListModel shellSurfaces: ListModel {}
    property bool isNestedCompositor: Qt.platform.pluginName.startsWith("wayland") || Qt.platform.pluginName === "xcb"
    property int currentActiveWindow: -1

    function handleShellSurface(shellSurface) {
        shellSurfaces.append({shellSurface: shellSurface});
    }

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

                Repeater {
                    id: chromeRepeater
                    model: output.shellSurfaces
                    // Chrome displays a shell surface on the screen (See Chrome.qml)
                    Chrome {
                        shellSurface: modelData
                        onDestroyAnimationFinished:
                        {
                            if (currentActiveWindow > index) {
                                --currentActiveWindow
                            } else if (currentActiveWindow === index) {
                                currentActiveWindow = index - 1
                                if (currentActiveWindow >= 0) {
                                    var nextActiveSurface = output.shellSurfaces.get(currentActiveWindow).shellSurface
                                    if (nextActiveSurface !== undefined) // More than one surface can get destroyed at the same time
                                        nextActiveSurface.active = true
                                }
                            }
                            output.shellSurfaces.remove(index)
                        }

                        onDeactivated: {
                            if (index === currentActiveWindow)
                                currentActiveWindow = -1
                        }

                        onActivated: {
                            if (index !== currentActiveWindow && currentActiveWindow >= 0) {
                                // This may already have been destroyed
                                if (output.shellSurfaces.get(currentActiveWindow).shellSurface !== undefined)
                                    output.shellSurfaces.get(currentActiveWindow).shellSurface.active = false
                            }

                            currentActiveWindow = index
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: taskbar
                color: "lavenderblush"
            }

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
                            modelData.requestWindowGeometry(modelData.windowState & ~Qt.WindowMinimized,
                                                            modelData.windowGeometry)
                            chromeRepeater.itemAt(index).activate()
                        }
                    }
                }
            }

            Item {
                id: usableArea
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: taskbar.top
            }

            // Virtual Keyboard
            Loader {
                anchors.fill: parent
                source: "Keyboard.qml"
            }

            // Draws the mouse cursor for a given Wayland seat
            WaylandCursorItem {
                inputEventsEnabled: false
                x: mouseTracker.mouseX
                y: mouseTracker.mouseY
                seat: output.compositor.defaultSeat
            }
        }

        Shortcut {
            sequence: "Ctrl+Alt+Backspace"
            onActivated: Qt.quit()
        }
    }
}
