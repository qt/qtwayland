/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

import QtQuick 2.0
import QtWayland.Compositor 1.0
import QtQuick.Window 2.11

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
        onCursorSurfaceRequest: {
            cursorItem.surface = surface;
            cursorItem.hotspotX = hotspotX;
            cursorItem.hotspotY = hotspotY;
        }
    }

    WaylandQuickItem {
        id: dragIcon
        property point offset

        x: cursorItem.hotspotX + offset.x
        y: cursorItem.hotspotY + offset.y
        z: -1
        surface: cursorItem.seat ? cursorItem.seat.drag.icon : null

        Connections {
            target: dragIcon.surface
            onOffsetForNextFrame: dragIcon.offset = offset;
        }
    }
}
