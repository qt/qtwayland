// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtWayland.Compositor

WaylandQuickItem {
    id: cursorItem
    property QtObject seat
    property int hotspotX: 0
    property int hotspotY: 0

    visible: cursorItem.surface != null
    inputEventsEnabled: false
    enabled: false
    transform: Translate {
        // If we've set an output scale factor different from the device pixel ratio
        // then the item will be rendered scaled, so we need to shift the hotspot accordingly
        x: -hotspotX * (output ? output.scaleFactor / Screen.devicePixelRatio : 1)
        y: -hotspotY * (output ? output.scaleFactor / Screen.devicePixelRatio : 1)
    }

    Connections {
        target: seat
        function onCursorSurfaceRequest(surface, hotspotX, hotspotY) {
            cursorItem.surface = surface;
            cursorItem.hotspotX = hotspotX;
            cursorItem.hotspotY = hotspotY;
        }
    }

    WaylandQuickItem {
        id: dragIcon
        property point offset
        inputEventsEnabled: false

        x: cursorItem.hotspotX + offset.x
        y: cursorItem.hotspotY + offset.y
        z: -1
        surface: cursorItem.seat ? cursorItem.seat.drag.icon : null

        Connections {
            target: dragIcon.surface
            function onOffsetForNextFrame(offset) {
                dragIcon.offset = offset;
            }
        }
    }
}
