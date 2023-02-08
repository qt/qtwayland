// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor

Item {
    id: chrome
    property alias shellSurface: surfaceItem.shellSurface
    property alias surfaceItem: surfaceItem
    property alias moveItem: surfaceItem.moveItem
    property alias output: surfaceItem.output

    //! [position sync]
    x: surfaceItem.moveItem.x - surfaceItem.output.geometry.x
    y: surfaceItem.moveItem.y - surfaceItem.output.geometry.y
    //! [position sync]

    ShellSurfaceItem {
        id: surfaceItem
        onSurfaceDestroyed: chrome.destroy();
    }

    onXChanged: updatePrimary()
    onYChanged: updatePrimary()
    function updatePrimary() {
        var w = surfaceItem.width
        var h = surfaceItem.height
        var area = w * h;
        var screenW = surfaceItem.output.geometry.width;
        var screenH = surfaceItem.output.geometry.height;
        var x1 = Math.max(0, x);
        var y1 = Math.max(0, y);
        var x2 = Math.min(x + w, screenW);
        var y2 = Math.min(y + h, screenH);
        var w1 = Math.max(0, x2 - x1);
        var h1 = Math.max(0, y2 - y1);
        if (w1 * h1 * 2 > area) {
            surfaceItem.setPrimary();
        }
    }
}
