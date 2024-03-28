// Copyright (C) 2015 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.4
import QtQuick.Window 2.2

Rectangle {
    id: root

    width: 300
    height: 300

    color: "blue"

    Rectangle {
        id: r
        width: 100
        height: 100
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        color: "cyan"

        MouseArea {
            anchors.fill: parent
            drag.target: r
            drag.axis: Drag.XAndYAxis
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: syncStatus.sync ? "sync mode" : "de-sync mode"
    }

    // If you can see these rectangles, something's not right
    Rectangle {
        //Child at (150, 70, 100, 100)
        color: "yellow"
        x: 150
        y: 70
        width:100
        height:100
    }
    Rectangle {
        //Shm at (30, 30, 50, 50)
        color: "yellow"
        x: 30
        y: 30
        width: 50
        height: 50
    }
}
