// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

WaylandOutput {
    id: output
    property alias gridSurfaces: listModel

    sizeFollowsWindow: true
    window: Window {
        width: 1024
        height: 760
        visible: true

        Image {
            id: background
            anchors.fill: parent
            fillMode: Image.Tile
            source: "qrc:/images/background.jpg"
            smooth: true
            GridView {
                id: gridView
                anchors.fill: parent
                model: ListModel {
                    id: listModel
                }
                interactive: false
                cellWidth: 200
                cellHeight: 200
                delegate: WaylandQuickItem {
                    id: item
                    surface: gridSurface
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                    inputEventsEnabled: false
                    allowDiscardFrontBuffer: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: item.surface.activated()
                    }
                }
            }
        }
    }

    XdgOutputV1 {
        name: "WL-2"
        description: "Overview screen"
        logicalPosition: output.position
        logicalSize: Qt.size(output.geometry.width / output.scaleFactor,
                             output.geometry.height / output.scaleFactor)
    }
}
