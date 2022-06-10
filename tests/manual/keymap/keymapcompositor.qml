// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0
import QtWayland.Compositor 1.0
import QtWayland.Compositor.WlShell
import QtQuick.Window 2.2

WaylandCompositor {
    id: wlcompositor
    property string layout: "no"

    defaultSeat.keymap {
        layout: layout
        options: "ctrl:swapcaps"
    }

    WaylandOutput {
        compositor: wlcompositor
        window: Window {
            width: 1024
            height: 768
            visible: true
            color: "#c17711"
            Text {
                anchors.centerIn: parent
                text: "Click the background to change layout: " + layout
            }
            MouseArea {
                anchors.fill: parent
                onClicked: layout = layout === "us" ? "no" : "us"
            }
            Item {
                id: surfaceArea
                anchors.fill: parent
            }
        }
    }
    Component {
        id: chromeComponent
        ShellSurfaceItem {
            onSurfaceDestroyed: destroy()
        }
    }
    WlShell {
        onWlShellSurfaceCreated: chromeComponent.createObject(surfaceArea, { "shellSurface": shellSurface } );
    }
}
