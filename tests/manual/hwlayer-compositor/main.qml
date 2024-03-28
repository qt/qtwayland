// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell
import QtWayland.Compositor.IviApplication

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            color: "tomato"
            id: win
            width: 1024
            height: 768
            visible: true
            Rectangle {
                color: "lightgreen"
                anchors.centerIn: parent
                width: parent.width / 3
                height: parent.width / 3
                NumberAnimation on rotation {
                    id: rotationAnimation
                    running: false
                    from: 0
                    to: 90
                    loops: Animation.Infinite
                    duration: 1000
                }
            }
            Repeater {
                model: shellSurfaces
                ShellSurfaceItem {
                    id: waylandItem
                    onSurfaceDestroyed: shellSurfaces.remove(index)
                    shellSurface: shSurface
                    WaylandHardwareLayer {
                        stackingLevel: level
                        Component.onCompleted: console.log("Added hardware layer with stacking level", stackingLevel);
                    }
                    Component.onCompleted: console.log("Added wayland quick item");
                    Behavior on x {
                        PropertyAnimation {
                            easing.type: Easing.OutBounce
                            duration: 1000
                        }
                    }
                    Timer {
                        interval: 2000; running: animatePosition; repeat: true
                        onTriggered: waylandItem.x = waylandItem.x === 0 ? win.width - waylandItem.width : 0
                    }
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 1000
                        }
                    }
                    Timer {
                        interval: 2000; running: animateOpacity; repeat: true
                        onTriggered: waylandItem.opacity = waylandItem.opacity === 1 ? 0 : 1
                    }
                }
            }
            Column {
                anchors.bottom: parent.bottom
                Repeater {
                    model: shellSurfaces
                    Row {
                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 15
                            rightPadding: 15
                            text: "Surface " + index
                        }
                        CheckBox {
                            text: "Animate position"
                            checked: animatePosition
                            onClicked: animatePosition = !animatePosition
                        }
                        CheckBox {
                            text: "Animate Opacity"
                            checked: animateOpacity
                            onClicked: animateOpacity = !animateOpacity
                        }
                        Label {
                            text: "Stacking level"
                        }
                        SpinBox {
                            value: level
                            onValueModified: level = value;
                        }
                        Button {
                            text: "Kill"
                            onClicked: shSurface.surface.client.kill()
                        }
                    }
                }
                CheckBox {
                    text: "Rotation"
                    checked: rotationAnimation.running
                    onClicked: rotationAnimation.running = !rotationAnimation.running
                    padding: 30
                }
            }
        }
    }
    ListModel { id: shellSurfaces }
    function addShellSurface(shellSurface) {
        shellSurfaces.append({shSurface: shellSurface, animatePosition: false, animateOpacity: false, level: 0});
    }
    XdgShell { onToplevelCreated: (toplevel, xdgSurface) => addShellSurface(xdgSurface) }
    IviApplication { onIviSurfaceCreated: (iviSurface) => addShellSurface(iviSurface) }
    WlShell { onWlShellSurfaceCreated: (shellSurface) => addShellSurface(shellSurface) }
}
