// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWayland.Compositor

WaylandOutput {
    id: output
    property alias surfaceArea: background
    sizeFollowsWindow: true

    window: Window {
        id: screen

        property QtObject output

        width: 1600
        height: 900
        visible: true

        Rectangle {
            id: sidebar
            width: 250
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: "lightgray"
            Column {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 5

                Repeater {
                    model: comp.itemList
                    Rectangle {
                        height: 54
                        width: sidebar.width - 5
                        color: "white"
                        radius: 5
                        Text {
                            text: "window: " + modelData.shellSurface.toplevel.title + "\n["
                                  + modelData.shellSurface.toplevel.appId
                                  + (modelData.isCustom ? "]\nfont size: " + modelData.fontSize : "]\nNo extension")
                            color: modelData.isCustom ? "black" : "darkgray"
                        }
                        MouseArea {
                            enabled: modelData.isCustom
                            anchors.fill: parent
                            onWheel: (wheel) => {
                                if (wheel.angleDelta.y > 0)
                                    modelData.fontSize++
                                else if (wheel.angleDelta.y < 0 && modelData.fontSize > 3)
                                    modelData.fontSize--
                            }
                            onDoubleClicked: {
                                output.compositor.customExtension.close(modelData.surface)
                            }
                        }
                    }
                }
                Text {
                    visible: comp.itemList.length > 0
                    width: sidebar.width - 5
                    text: "Mouse wheel to change font size. Double click to close"
                    wrapMode: Text.Wrap
                }
            }
        }

        WaylandMouseTracker {
            id: mouseTracker
            anchors.left: sidebar.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            windowSystemCursorEnabled: !clientCursor.visible
            Image {
                id: background
                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.png"
                smooth: false
            }
            WaylandCursorItem {
                id: clientCursor
                x: mouseTracker.mouseX
                y: mouseTracker.mouseY

                seat: output.compositor.defaultSeat
            }

            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                width: 100
                height: 100
                property bool on : true
                color: on ? "#DEC0DE" : "#FACADE"
                Text {
                    anchors.fill: parent
                    text: "Toggle window decorations"
                    wrapMode: Text.WordWrap
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        parent.on = !parent.on
                        comp.setDecorations(parent.on);
                    }
                }
            }
        }
    }
}
