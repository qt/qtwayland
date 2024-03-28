// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.WlShell
import QtQuick.Window

import io.qt.examples.sharebufferextension

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true
            Image {
                id: surfaceArea
                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.png"
                smooth: false
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
        onWlShellSurfaceCreated: (shellSurface) => {
            chromeComponent.createObject(surfaceArea, { "shellSurface": shellSurface } );
        }
    }

    ShareBufferExtension {
    }
}
