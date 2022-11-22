// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

WaylandCompositor {
    // The output defines the screen.
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true
            Rectangle {
                anchors.fill: parent
                gradient: "MorpheusDen"
            }

            Repeater {
                model: shellSurfaces

                // ![decoration]
                Column {
                    id: chrome
                    width: shellSurfaceItem.implicitWidth
                    Rectangle {
                        visible: modelData.toplevel.decorationMode === XdgToplevel.ServerSideDecoration
                        width: parent.width
                        height: 30
                        gradient: "HeavyRain";
                        Text {
                            text: modelData.toplevel.title
                            anchors.centerIn: parent
                        }
                        Item {
                            anchors.right: parent.right
                            width: 30
                            height: 30
                            Text { text: "X"; anchors.centerIn: parent }
                            TapHandler {
                                onTapped: modelData.toplevel.sendClose()
                            }
                        }
                        DragHandler {
                            target: chrome
                        }
                    }
                    ShellSurfaceItem {
                        id: shellSurfaceItem
                        moveItem: parent
                        shellSurface: modelData
                        onSurfaceDestroyed: shellSurfaces.remove(index)
                    }
                }
                // ![decoration]
            }
        }
    }

    // ![XdgShell]
    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => shellSurfaces.append({shellSurface: xdgSurface});
    }
    XdgDecorationManagerV1 {
        preferredMode: XdgToplevel.ServerSideDecoration
    }
    // ![XdgShell]
    ListModel { id: shellSurfaces }
}
