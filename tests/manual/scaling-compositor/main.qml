// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.15
import QtQuick.Window 2.2
import QtWayland.Compositor 1.3
import QtWayland.Compositor.WlShell
import QtWayland.Compositor.XdgShell

WaylandCompositor {
    id: comp
    WaylandOutput {
        id: output
        compositor: comp
        sizeFollowsWindow: true
        scaleFactor: 2
        window: Window {
            id: win
            width: 500
            height: 500
            visible: true
            title: "Scaling compositor x" + output.scaleFactor

            Repeater {
                model: shellSurfaces
                ShellSurfaceItem {
                    shellSurface: modelData
                    onSurfaceDestroyed: shellSurfaces.remove(index);
                }
            }

            Rectangle {
                id: incrementButton
                color: "#c0f0d0"
                Text {
                    text: "+"
                }
                width: 100
                height: 30
                TapHandler {
                    onTapped: ++output.scaleFactor
                }
            }

            Rectangle {
                id: decrementButton
                color: "#f0d0c0"
                Text {
                    text: "-"
                }
                width: 100
                height: 30
                TapHandler {
                    onTapped: output.scaleFactor = Math.max(1, output.scaleFactor - 1)
                }
                anchors.left: incrementButton.right
            }
        }
    }

    ListModel { id: shellSurfaces }

    WlShell {
        onWlShellSurfaceCreated: shellSurfaces.append({shellSurface: shellSurface});
    }
    XdgShell {
        onToplevelCreated:
            shellSurfaces.append({shellSurface: xdgSurface});
    }
}
