// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

DragHandler {
    target: null

    function selectCursor(f)
    {
        switch (f) {
        case (southBound | eastBound):
        case (northBound | westBound):
            return Qt.SizeFDiagCursor
        case (southBound | westBound):
        case (northBound | eastBound):
            return Qt.SizeBDiagCursor
        case westBound:
        case eastBound:
            return Qt.SizeHorCursor
        default:
            return Qt.SizeVerCursor
        }
    }

    property int flags: WestBound
    readonly property int westBound: 1
    readonly property int eastBound: 2
    readonly property int northBound: 4
    readonly property int southBound: 8

    cursorShape: selectCursor(flags)

    property rect geom
    property real startX: -1.0
    property real startY: -1.0
    property bool started: false
    onGrabChanged: {
        started = false
    }
    onCentroidChanged: {
        if (!active)
            return
        if (!started) {
            geom = shellSurface.windowGeometry
            startX = centroid.scenePressPosition.x
            startY = centroid.scenePressPosition.y
            started = true
        }

        var pos = chrome.constrainPoint(centroid.scenePosition)
        var dx = pos.x - startX
        var dy = pos.y - startY

        var minWidth = Math.max(0, shellSurface.minimumSize.width)
        var minHeight = Math.max(0, shellSurface.minimumSize.height)

        var maxWidth = shellSurface.maximumSize.width > 0 ? shellSurface.maximumSize.width : Number.MAX_VALUE
        var maxHeight = shellSurface.maximumSize.height > 0 ? shellSurface.maximumSize.height : Number.MAX_VALUE

        var newLeft = geom.left
        if (flags & westBound)
            newLeft = Math.max(geom.right - maxWidth, Math.min(geom.right - minWidth, newLeft + dx));

        var newTop =  geom.top
        if (flags & northBound)
            newTop = Math.max(geom.bottom - maxHeight, Math.min(geom.bottom - minHeight, newTop + dy));

        var newRight = geom.right
        if (flags & eastBound)
            newRight = Math.max(geom.left, newRight + dx);

        var newBottom = geom.bottom
        if (flags & southBound)
            newBottom = Math.max(geom.top, newBottom + dy);

        console.log("RESIZE HANDLER", shellSurface.windowGeometry, geom, dy, newTop)


        shellSurface.requestWindowGeometry(shellSurface.windowState,
                                           Qt.rect(newLeft,
                                              newTop,
                                              newRight - newLeft,
                                              newBottom - newTop))
    }
}
