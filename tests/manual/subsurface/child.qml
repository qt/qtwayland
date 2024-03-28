// Copyright (C) 2015 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.4
import QtQuick.Window 2.2

Rectangle {
    width: 300
    height: 300

    color: "darkgray"

    Rectangle {
        width: 100
        height: 100
        color: "magenta"

        Text {
            anchors.centerIn: parent
            text: "quick"
        }

        RotationAnimation on rotation {
            duration: 10000
            loops: Animation.Infinite
            from: 0
            to: 360
        }
    }
}
