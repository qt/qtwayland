// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

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
