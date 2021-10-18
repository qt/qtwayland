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
import QtQuick.Controls
import QtQuick.Layouts
import QtWayland.Compositor
import QtWayland.Compositor.QtShell

QtShellChrome {
    id: chrome

    property alias shellSurface: shellSurfaceItemId.shellSurface

    //! [maximizedRect]
    maximizedRect: Qt.rect(usableArea.x,
                           usableArea.y,
                           usableArea.width,
                           usableArea.height)
    //! [maximizedRect]

    //! [leftResizeHandle]
    Rectangle {
        id: leftResizeHandle
        color: "gray"
        width: visible ? 5 : 0
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }
    //! [leftResizeHandle]

    Rectangle {
        id: rightResizeHandle
        color: "gray"
        width: visible ? 5 : 0
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }

    Rectangle {
        id: topResizeHandle
        color: "gray"
        height: visible ? 5 : 0
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
    }

    Rectangle {
        id: bottomResizeHandle
        color: "gray"
        height: visible ? 5 : 0
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }

    Rectangle {
        id: titleBar
        anchors.top: topResizeHandle.bottom
        anchors.left: leftResizeHandle.right
        anchors.right: rightResizeHandle.left
        height: visible ? xButton.height + 10 : 0
        color: shellSurface.active ? "cornflowerblue" : "lightgray"

        Text {
            anchors.left: parent.left
            anchors.right: rowLayout.left
            anchors.verticalCenter: parent.verticalCenter

            font.pixelSize: xButton.height
            text: shellSurface.windowTitle
            fontSizeMode: Text.Fit
        }

        //! [buttons]
        RowLayout {
            id: rowLayout
            anchors.right: parent.right
            anchors.rightMargin: 5

            ToolButton {
                text: "-"
                Layout.margins: 5
                visible: (chrome.windowFlags & Qt.WindowMinimizeButtonHint) != 0
                onClicked: {
                    chrome.toggleMinimized()
                }
            }

            ToolButton {
                text: "+"
                Layout.margins: 5
                visible: (chrome.windowFlags & Qt.WindowMaximizeButtonHint) != 0
                onClicked: {
                    chrome.toggleMaximized()
                }
            }

            ToolButton {
                id: xButton
                text: "X"
                Layout.margins: 5
                visible: (chrome.windowFlags & Qt.WindowCloseButtonHint) != 0
                onClicked: shellSurface.sendClose()
            }
        }
        //! [buttons]
    }

    Rectangle {
        id: topLeftResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.left: parent.left
        anchors.top: parent.top
    }

    Rectangle {
        id: topRightResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.right: parent.right
        anchors.top: parent.top
    }

    Rectangle {
        id: bottomLeftResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Rectangle {
        id: bottomRightResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    //! [decorations]
    leftResizeHandle: leftResizeHandle
    rightResizeHandle: rightResizeHandle
    topResizeHandle: topResizeHandle
    bottomResizeHandle: bottomResizeHandle
    bottomLeftResizeHandle: bottomLeftResizeHandle
    bottomRightResizeHandle: bottomRightResizeHandle
    topLeftResizeHandle: topLeftResizeHandle
    topRightResizeHandle: topRightResizeHandle
    titleBar: titleBar
    //! [decorations]

    //! [shellsurfaceitem]
    ShellSurfaceItem {
        id: shellSurfaceItemId
        anchors.top: titleBar.bottom
        anchors.bottom: bottomResizeHandle.top
        anchors.left: leftResizeHandle.right
        anchors.right: rightResizeHandle.left

        moveItem: chrome

        staysOnBottom: shellSurface.windowFlags & Qt.WindowStaysOnBottomHint
        staysOnTop: !staysOnBottom && shellSurface.windowFlags & Qt.WindowStaysOnTopHint
    }
    shellSurfaceItem: shellSurfaceItemId
    //! [shellsurfaceitem]
}
