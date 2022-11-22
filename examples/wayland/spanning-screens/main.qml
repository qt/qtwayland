// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            id: topSurfaceArea

            property int pixelHeight: screen.devicePixelRatio * height
            property int pixelWidth: screen.devicePixelRatio * width

            width: 1024
            height: 768
            visible: true
            color: "#1337af"

            Text { text: "Top screen" }

            // ![enable screens]
            // Enable the following to make the output target an actual screen,
            // for example when running on eglfs in a multi-display embedded system.

            // screen: Qt.application.screens[0]
            // ![enable screens]
        }
    }

    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            id: bottomSurfaceArea

            property int pixelHeight: screen.devicePixelRatio * height
            property int pixelWidth: screen.devicePixelRatio * width

            width: 1024
            height: 768
            visible: true
            color: "#1abacc"

            Text { text: "Bottom screen" }

            // Enable the following to make the output target an actual screen,
            // for example when running on eglfs in a multi-display embedded system.

            // screen: Qt.application.screens[1]
        }
    }

    Component {
        id: chromeComponent
        Item {
            property alias shellSurface: ssi.shellSurface
            ShellSurfaceItem {
                id: ssi
                onSurfaceDestroyed: destroy()
            }
        }
    }

    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => {
            const shellSurface = xdgSurface;

            // ![create items]
            const topItem = chromeComponent.createObject(topSurfaceArea, {
                shellSurface
            });

            const bottomItem = chromeComponent.createObject(bottomSurfaceArea, {
                shellSurface,
                y: Qt.binding(function() { return -topSurfaceArea.height;})
            });
            // ![create items]

            // ![size]
            const height = topSurfaceArea.pixelHeight + bottomSurfaceArea.pixelHeight;
            const width = Math.max(bottomSurfaceArea.pixelWidth, topSurfaceArea.pixelWidth);
            const size = Qt.size(width, height);
            toplevel.sendFullscreen(size);
            // ![size]
        }
    }
}
