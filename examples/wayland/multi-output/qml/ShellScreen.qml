// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

WaylandOutput {
    id: output
    property alias surfaceArea: background

    sizeFollowsWindow: true
    window: Window {
        width: 1024
        height: 760
        visible: true

        WaylandMouseTracker {
            id: mouseTracker
            anchors.fill: parent

            windowSystemCursorEnabled: !clientCursor.visible
            Image {
                id: background
                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.jpg"
                smooth: true
            }
            WaylandCursorItem {
                id: clientCursor
                x: mouseTracker.mouseX
                y: mouseTracker.mouseY
                visible: surface != null && mouseTracker.containsMouse
                seat : output.compositor.defaultSeat
            }
        }
    }

    XdgOutputV1 {
        name: "WL-1"
        description: "Screen with window management"
        logicalPosition: output.position
        logicalSize: Qt.size(output.geometry.width / output.scaleFactor,
                             output.geometry.height / output.scaleFactor)
    }
}
