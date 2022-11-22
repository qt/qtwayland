// Copyright (C) 2017 Erik Larsson.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import io.qt.examples.customextension

Window {
    id: topLevelWindow

    property alias textItem: bounceText

    title: "QML Client"
    visible: true

    Rectangle {
        anchors.fill: parent
        color: "#f1eece"
    }

    Text {
        id: bounceText
        text: "press here to bounce"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (customExtension.active)
                customExtension.sendBounce(topLevelWindow, 1000)
        }
    }

    MouseArea {
        anchors.centerIn: parent
        width: 100; height: 100
        onClicked: {
            if (customExtension.active)
                customExtension.sendSpin(topLevelWindow, 500)
        }

        Rectangle {
            anchors.fill: parent
            color: "#fab1ed"
            Text {
                text: "spin"
            }
        }
    }

    CustomExtension {
        id: customExtension
        onActiveChanged: {
            console.log("Custom extension is active:", active)
            registerWindow(topLevelWindow)
        }
        onFontSize: (window, pixelSize) => {
            // signal arguments: window and pixelSize
            // we are free to interpret the protocol as we want, so
            // let's change the font size of just one of the text items
            window.textItem.font.pixelSize = pixelSize
        }
    }
}

