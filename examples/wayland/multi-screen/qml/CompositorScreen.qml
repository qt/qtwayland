// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtQuick.Window

WaylandOutput {
    id: screen
    property variant viewsBySurface: ({})
    property alias surfaceArea: background
    property alias text: t.text
    property alias screen: win.screen
    sizeFollowsWindow: true

    property bool windowed: false

    window: Window {
        id: win
        x: Screen.virtualX
        y: Screen.virtualY
        width: 800
        height: 800
        visibility: windowed ? Window.Windowed : Window.FullScreen
        visible: true

        WaylandMouseTracker {
            id: mouseTracker
            anchors.fill: parent
            windowSystemCursorEnabled: !clientCursor.visible

            Rectangle {
                anchors.fill: parent
                id: background

                Text {
                    id: t
                    anchors.centerIn: parent
                    font.pointSize: 72
                }
            }

            WaylandCursorItem {
                id: clientCursor
                inputEventsEnabled: false
                x: mouseTracker.mouseX
                y: mouseTracker.mouseY
                seat: comp.defaultSeat
                visible: surface != null && mouseTracker.containsMouse
            }
        }
        Shortcut {
            sequence: "Ctrl+Alt+Backspace"
            onActivated: Qt.quit()
        }
    }
}
