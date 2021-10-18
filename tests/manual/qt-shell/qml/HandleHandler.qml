/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
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
