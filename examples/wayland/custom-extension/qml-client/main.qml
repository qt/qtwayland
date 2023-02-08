// Copyright (C) 2017 Erik Larsson.
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import io.qt.examples.customextension

Window {
    id: topLevelWindow
    title: "QML Client"
    visible: true
    width: 800
    height: 600

    property real fontSize: height / 50

    Column {
        anchors.centerIn: parent
        width: topLevelWindow.width / 4
        height: 2 * (width + topLevelWindow.height / 12)
        spacing: topLevelWindow.height / 12

        Rectangle {
            id: rect1
            color: "#C0FFEE"
            width: parent.width
            height: width
            clip: true

            Text {
                anchors.centerIn: parent
                text: "Press here to send spin request."
                font.pixelSize: fontSize
                width: parent.width
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            TapHandler {
                onTapped: {
                    if (customExtension.active)
                        customExtension.sendSpin(topLevelWindow, 500)
                }
            }
        }

        Rectangle {
            id: rect2
            color: "#decaff"
            width: parent.width
            height: parent.height / 2
            clip: true

            Text {
                anchors.centerIn: parent
                text: "Press here to send bounce request."
                font.pixelSize: fontSize
                width: parent.width
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

//! [sendBounce]
            TapHandler {
                onTapped: {
                    if (customExtension.active)
                        customExtension.sendBounce(topLevelWindow, 1000)
                }
            }
//! [sendBounce]
        }
    }

//! [CustomExtension]
    CustomExtension {
        id: customExtension
        onActiveChanged: {
            registerWindow(topLevelWindow)
        }
        onFontSize: (window, pixelSize) => {
            topLevelWindow.fontSize = pixelSize
        }
    }
//! [CustomExtension]
}

