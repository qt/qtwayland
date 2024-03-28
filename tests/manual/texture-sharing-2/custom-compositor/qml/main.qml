// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell

import io.qt.tests.customsharingextension

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true
            Image {
                id: background
                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.png"
                smooth: true

                Rectangle {
                    width: 100
                    height: 100
                    color: "red"
                    anchors.bottom: parent.bottom;
                    anchors.right: parent.right;
                    MouseArea {
                        anchors.fill: parent
                        onClicked: sharedTextureImage.source = "image://wlshared/car.ktx"
                    }
                }
                Image {
                    id: sharedTextureImage
                    anchors.bottom: parent.bottom;
                    anchors.right: parent.right;
                    source: ""
                }
                Image {
                    id: topRightImage
                    anchors.top: parent.top;
                    anchors.right: parent.right;
                    source: "image://wlshared/qt_logo.png"
                }
            }
            Repeater {
                model: shellSurfaces
                ShellSurfaceItem {
                    shellSurface: modelData
                    onSurfaceDestroyed: shellSurfaces.remove(index)
                }
            }
        }
    }
    WlShell {
        onWlShellSurfaceCreated:
            shellSurfaces.append({shellSurface: shellSurface});
    }
    XdgShell {
        onToplevelCreated:
            shellSurfaces.append({shellSurface: xdgSurface});
    }
    ListModel { id: shellSurfaces }

    CustomSharingExtension {
        imageSearchPath: ":/images;."
    }
}
