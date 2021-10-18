/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Wayland module
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtWayland.Compositor

WaylandOutput {
    id: output

    property bool isNestedCompositor: Qt.platform.pluginName.startsWith("wayland") || Qt.platform.pluginName === "xcb"

    //! [handleShellSurface]
    property ListModel shellSurfaces: ListModel {}
    function handleShellSurface(shellSurface) {
        shellSurfaces.append({shellSurface: shellSurface});
    }
    //! [handleShellSurface]

    // During development, it can be useful to start the compositor inside X11 or
    // another Wayland compositor. In such cases, set sizeFollowsWindow to true to
    // enable resizing of the compositor window to be forwarded to the Wayland clients
    // as the output (screen) changing resolution. Consider setting it to false if you
    // are running the compositor using eglfs, linuxfb or similar QPA backends.
    sizeFollowsWindow: output.isNestedCompositor

    window: Window {
        width: 1920
        height: 1080
        visible: true

        WaylandMouseTracker {
            id: mouseTracker

            anchors.fill: parent

            // Set this to false to disable the outer mouse cursor when running nested
            // compositors. Otherwise you would see two mouse cursors, one for each compositor.
            windowSystemCursorEnabled: output.isNestedCompositor

            Image {
                id: background

                anchors.fill: parent
                fillMode: Image.Tile
                source: "qrc:/images/background.jpg"
                smooth: true

                //! [repeater]
                Repeater {
                    id: chromeRepeater
                    model: output.shellSurfaces
                    // Chrome displays a shell surface on the screen (See Chrome.qml)
                    Chrome {
                        shellSurface: modelData
                        onClientDestroyed:
                        {
                            output.shellSurfaces.remove(index)
                        }
                    }
                }
                //! [repeater]
            }

            Rectangle {
                anchors.fill: taskbar
                color: "lavenderblush"
            }

            //! [taskbar]
            Row {
                id: taskbar
                height: 40
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                Repeater {
                    anchors.fill: parent
                    model: output.shellSurfaces

                    ToolButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.windowTitle
                        onClicked: {
                            var item = chromeRepeater.itemAt(index)
                            if ((item.windowState & Qt.WindowMinimized) != 0)
                                item.toggleMinimized()
                            chromeRepeater.itemAt(index).activate()
                        }
                    }
                }
            }
            //! [taskbar]

            //! [usableArea]
            Item {
                id: usableArea
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: taskbar.top
            }
            //! [usableArea]
        }
    }
}
