// Copyright (C) 2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.2
import QtQuick.Window 2.2

Window {
    id: window
    width: 400
    height: 400
    color: "blue"
    visible: true

    Column {
        spacing: 8

        Row {
            spacing: 8

            Repeater {
                model: ListModel {
                    ListElement { label: "Windowed"; value: Window.Windowed }
                    ListElement { label: "Maximized"; value: Window.Maximized }
                    ListElement { label: "FullScreen"; value: Window.FullScreen }
                }

                Rectangle {
                    width: 96
                    height: 40
                    color: "gainsboro"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: window.visibility = model.value

                        Text {
                            anchors.centerIn: parent
                            text: model.label
                        }
                    }
                }
            }
        }

        Text {
            color: "white"
            text: {
                switch (window.visibility) {
                case Window.Windowed:
                    return "windowed";
                case Window.Maximized:
                    return "maximized";
                case Window.FullScreen:
                    return "fullscreen";
                case Window.Minimized:
                    return "minimized";
                case Window.AutomaticVisibility:
                    return "automatic";
                case Window.Hidden:
                    return "hidden";
                default:
                    break;
                }
                return "unknown";
            }
        }
    }
}
