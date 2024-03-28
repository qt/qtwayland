// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWayland.Compositor

Item {
    id: chrome

    property bool positionSet: false

    x: shellSurface.windowGeometry.x - leftResizeHandle.width
    y: shellSurface.windowGeometry.y - topResizeHandle.height - titleBar.height
    width: shellSurface.windowGeometry.width + leftResizeHandle.width + rightResizeHandle.width
    height: shellSurface.windowGeometry.height + topResizeHandle.height + titleBar.height + bottomResizeHandle.height

    property rect oldGeometry: Qt.rect(0, 0, 100, 100)
    property bool isChild: parent.shellSurface !== undefined
    property alias shellSurface: shellSurfaceItem.shellSurface

    property int windowState: Qt.WindowNoState

    signal destroyAnimationFinished
    signal activated
    signal deactivated

    property int windowFlags: shellSurface.windowFlags !== Qt.Window
                                ? shellSurface.windowFlags
                                : defaultFlags
    onDecorationsShowingChanged:{
        shellSurfaceItem.updateFrameMargins()
    }

    Component.onCompleted: {
        shellSurface.active = true
    }

    property int defaultFlags: (Qt.Window
                                | Qt.WindowMaximizeButtonHint
                                | Qt.WindowMinimizeButtonHint
                                | Qt.WindowCloseButtonHint)

    property bool frameless: (chrome.windowFlags & Qt.FramelessWindowHint) != 0
                             || (chrome.windowState & Qt.WindowFullScreen) != 0
                             || ((chrome.windowFlags & Qt.Popup) == Qt.Popup
                                 && (chrome.windowFlags & Qt.Tool) != Qt.Tool)

    property bool decorationsShowing: (chrome.windowFlags & Qt.Window) != 0 && !frameless

    transform: [
        Scale {
            id: scaleTransform
            origin.x: chrome.width / 2
            origin.y: chrome.height / 2
        }
    ]

    Rectangle {
        id: leftResizeHandle
        color: "gray"
        width: visible ? 5 : 0
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: westBound
        }
    }

    Rectangle {
        id: rightResizeHandle
        color: "gray"
        width: visible ? 5 : 0
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: eastBound
        }
    }

    Rectangle {
        id: topResizeHandle
        color: "gray"
        height: visible ? 5 : 0
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: northBound
        }
    }

    Rectangle {
        id: bottomResizeHandle
        color: "gray"
        height: visible ? 5 : 0
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: southBound
        }
    }

    Rectangle {
        id: topLeftResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.left: parent.left
        anchors.top: parent.top
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: westBound | northBound
        }
    }

    Rectangle {
        id: topRightResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.right: parent.right
        anchors.top: parent.top
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: eastBound | northBound
        }
    }

    Rectangle {
        id: bottomLeftResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: westBound | southBound
        }
    }

    Rectangle {
        id: bottomRightResizeHandle
        color: "gray"
        height: 5
        width: 5
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: decorationsShowing

        HandleHandler {
            enabled: (chrome.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0
            flags: eastBound | southBound
        }
    }

    function constrainPoint(mousePos) {
        var x0 = usableArea.x
        var y0 = usableArea.y
        var x1 = x0 + usableArea.width
        var y1 = y0 + usableArea.height
        return Qt.point(Math.min(Math.max(x0,mousePos.x), x1),
                        Math.min(Math.max(y0,mousePos.y), y1))
    }

    function maxContentRect() {
        var x0 = usableArea.x + leftResizeHandle.width
        var x1 = usableArea.x + usableArea.width - rightResizeHandle.width
        var y0 = usableArea.y + topResizeHandle.height + titleBar.height
        var y1 = usableArea.y + usableArea.height - bottomResizeHandle.height
        return Qt.rect(x0, y0, x1 - x0, y1 - y0)
    }

    function randomPos(windowSize, screenSize) {
        var res = (windowSize >= screenSize) ? 0 : Math.floor(Math.random() * (screenSize - windowSize))
        return res
    }

    function activate()
    {
        shellSurface.active = true
        shellSurfaceItem.raise()
        activated()
    }

    function deactivate()
    {
        shellSurface.active = true
        deactivated()
    }

    function setWindowState(nextState) {
        var currentState = chrome.windowState
        if (currentState === nextState)
            return

        console.log("setWindowState", nextState.toString(16))

        if ((currentState & (Qt.WindowMinimized | Qt.WindowMaximized | Qt.WindowFullScreen)) == 0)
            chrome.oldGeometry = chrome.shellSurface.windowGeometry

        chrome.windowState = nextState

        if ((nextState & Qt.WindowMinimized) != 0) {
            console.log("MINIMIZE")
            chrome.shellSurface.requestWindowGeometry(nextState, Qt.rect(0, 0, 1, 1))
            shellSurfaceItem.visible = false
        } else if ((nextState & Qt.WindowFullScreen) != 0) {
            console.log("FULLSCREENIZE")
            chrome.shellSurface.requestWindowGeometry(nextState, Qt.rect(0, 0, output.window.width, output.window.height))
            shellSurfaceItem.visible = true
        } else if ((nextState & Qt.WindowMaximized) != 0) {
            console.log("MAXIMIZE")
            chrome.shellSurface.requestWindowGeometry(nextState, maxContentRect())
            shellSurfaceItem.visible = true
        } else {
            console.log("NORMALIZE", chrome.oldGeometry)
            chrome.shellSurface.requestWindowGeometry(nextState, chrome.oldGeometry)
            shellSurfaceItem.visible = true
        }
    }

    Rectangle {
        id: titleBar
        anchors.top: topResizeHandle.bottom
        anchors.left: leftResizeHandle.right
        anchors.right: rightResizeHandle.left
        height: visible ? xButton.height + 10 : 0
        color: shellSurface.active ? "cornflowerblue" : "lightgray"
        visible: !frameless

        Text {
            anchors.left: parent.left
            anchors.right: rowLayout.left
            anchors.verticalCenter: parent.verticalCenter

            font.pixelSize: xButton.height
            text: shellSurface.windowTitle
            fontSizeMode: Text.Fit
        }

        RowLayout {
            id: rowLayout
            anchors.right: parent.right
            anchors.rightMargin: 5

            ToolButton {
                text: "-"
                Layout.margins: 5
                visible: (chrome.windowFlags & Qt.WindowMinimizeButtonHint) != 0
                onClicked: {
                    var newState
                    if ((shellSurface.windowState & Qt.WindowMinimized) != 0)
                        newState = chrome.windowState & ~Qt.WindowMinimized
                    else
                        newState = chrome.windowState | Qt.WindowMinimized

                    if ((newState & Qt.WindowMaximized) != 0)
                        newState &= ~Qt.WindowMaximized

                    setWindowState(newState)
                }
            }

            ToolButton {
                text: "+"
                Layout.margins: 5
                visible: (chrome.windowFlags & Qt.WindowMaximizeButtonHint) != 0
                onClicked: {
                    var newState
                    if ((shellSurface.windowState & Qt.WindowMaximized) != 0)
                        newState = shellSurface.windowState & ~Qt.WindowMaximized
                    else
                        newState = shellSurface.windowState | Qt.WindowMaximized

                    if ((newState & Qt.WindowMinimized) != 0)
                        newState &= ~Qt.WindowMinimized

                    setWindowState(newState)
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

        DragHandler {
            target: null
            property real xOffset: -1.0
            property real yOffset: -1.0
            property bool started: false
            enabled: (shellSurface.windowState & (Qt.WindowMinimized|Qt.WindowMaximized)) == 0

            onGrabChanged: {
                started = false
                activate()
            }

            onCentroidChanged: {
                if (!active)
                    return

                if (!started) {
                    xOffset = shellSurface.windowPosition.x - centroid.scenePressPosition.x
                    yOffset = shellSurface.windowPosition.y - centroid.scenePressPosition.y
                    started = true
                    chrome.positionAutomatic = false
                }

                var pos = chrome.constrainPoint(centroid.scenePosition)
                shellSurface.windowPosition = Qt.point(pos.x + xOffset, pos.y + yOffset)
            }
        }
    }

    ShellSurfaceItem {
        id: shellSurfaceItem
        anchors.top: titleBar.bottom
        anchors.bottom: bottomResizeHandle.top
        anchors.left: leftResizeHandle.right
        anchors.right: rightResizeHandle.left

        moveItem: chrome

        staysOnBottom: shellSurface.windowFlags & Qt.WindowStaysOnBottomHint
        staysOnTop: !staysOnBottom && shellSurface.windowFlags & Qt.WindowStaysOnTopHint
        function updateFrameMargins()
        {
            shellSurface.frameMarginLeft = (decorationsShowing ? leftResizeHandle.width : 0)
            shellSurface.frameMarginRight = (decorationsShowing ? rightResizeHandle.width : 0)
            shellSurface.frameMarginTop = (decorationsShowing ? topResizeHandle.height : 0)
                           + (!frameless ? titleBar.height : 0)
            shellSurface.frameMarginBottom = (decorationsShowing ? bottomResizeHandle.height : 0)
        }

        Component.onCompleted: {
            updateFrameMargins()
        }

        onSurfaceDestroyed: {
            bufferLocked = true;
            destroyAnimation.start();
        }

        Connections {
            target: shellSurface
            function onWindowFlagsChanged() {
                console.log("FLAGS", shellSurface.windowFlags.toString(16))
                shellSurfaceItem.updateFrameMargins()
            }

            function onWindowStateChanged() {
                setWindowState(shellSurface.windowState)
            }

            function onActiveChanged() {
                if (shellSurface.active) {
                    shellSurfaceItem.raise()
                    activated()
                } else {
                    deactivated()
                }
            }

            function onStartResize() {
                console.log("START SYSTEM RESIZE")
            }
            function onStartMove() {
                console.log("START SYSTEM MOVE")
            }

            function onRaiseRequested() {
                console.log("RAISE")
                shellSurfaceItem.raise()
            }
            function onLowerRequested() {
                console.log("LOWER")
                shellSurfaceItem.lower()
            }

            function onWindowGeometryChanged() {
                console.log("GEOM CHANGE", shellSurface.windowGeometry)
            }
        }

        Connections {
          target: shellSurface.surface
            function onHasContentChanged() {
                if (!chrome.positionSet) {
                    var rect = shellSurface.windowGeometry
                    var w = rect.width
                    var h = rect.height

                    var space = maxContentRect()

                    var randomize = shellSurface.positionAutomatic
                    var xpos = randomize ? randomPos(w, space.width) + space.x : Math.max(rect.x, space.x)
                    var ypos = randomize ? randomPos(h, space.height) + space.y : Math.max(rect.y, space.y)
                    shellSurface.windowPosition = Qt.point(xpos, ypos)
                }
                chrome.positionSet = true
            }
        }

        SequentialAnimation {
            id: destroyAnimation

            ParallelAnimation {
                NumberAnimation { target: scaleTransform; property: "yScale"; to: 2/height; duration: 150 }
                NumberAnimation { target: scaleTransform; property: "xScale"; to: 0.4; duration: 150 }
                NumberAnimation { target: chrome; property: "opacity"; to: chrome.isChild ? 0 : 1; duration: 150 }
            }
            NumberAnimation { target: scaleTransform; property: "xScale"; to: 0; duration: 150 }
            ScriptAction { script: chrome.destroyAnimationFinished() }
        }

        SequentialAnimation {
            id: receivedFocusAnimation

            ParallelAnimation {
                NumberAnimation { target: scaleTransform; property: "yScale"; to: 1.02; duration: 100; easing.type: Easing.OutQuad }
                NumberAnimation { target: scaleTransform; property: "xScale"; to: 1.02; duration: 100; easing.type: Easing.OutQuad }
            }
            ParallelAnimation {
                NumberAnimation { target: scaleTransform; property: "yScale"; to: 1; duration: 100; easing.type: Easing.InOutQuad }
                NumberAnimation { target: scaleTransform; property: "xScale"; to: 1; duration: 100; easing.type: Easing.InOutQuad }
            }
        }
    }
}
