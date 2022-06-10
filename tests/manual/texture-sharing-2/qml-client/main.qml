// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window

import QtWayland.Client.TextureSharing

Window {
    width: 800
    height: 500
    visible: true

    Rectangle {
        anchors.fill: parent
        color: "#C0FEFE"

        Flickable {
            anchors.fill: parent
            contentHeight: imageGrid.height

            Grid {
                id: imageGrid
                columns: 2
                width: parent.width
                spacing: 25
                padding: 25


                // loadedImage
                Text {
                    width: 400
                    wrapMode: Text.Wrap
                    text: "An Image element using the shared buffer provider to load from a PNG image.<br>" +
                          "Source: '" + loadedImage.source + "'" +
                          (loadedImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                }
                Image {
                    id: loadedImage
                    fillMode: Image.PreserveAspectFit
                    source: "image://wlshared/qt_logo.png"
                }
                Rectangle {
                    visible: loadedImage.height <= 0
                    width:100; height: 100
                    color: "green"
                }

                // paintedImage
                Text {
                    width: 400
                    wrapMode: Text.Wrap
                    text: "An Image element using the shared buffer provider.<br>" +
                          "This texture is created by the compositor using QPainter. <br>" +
                          "Source: '" + paintedImage.source + "'" +
                          (paintedImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                }
                Image {
                    id: paintedImage
                    fillMode: Image.PreserveAspectFit
                    source: "image://wlshared/test pattern 1"
                }
                Rectangle {
                    visible: paintedImage.height <= 0
                    width:100; height: 100
                    color: "green"
                }

                // ktxImage
                Text {
                    width: 400
                    wrapMode: Text.Wrap
                    text: "An Image element using the shared buffer provider to load an ETC2 compressed texture." +
                          "<br>Source: '" + ktxImage.source + "'" +
                          (ktxImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                }
                Image {
                    id: ktxImage
                    source: "image://wlshared/car.ktx"
                    fillMode: Image.PreserveAspectFit
                }
                Rectangle {
                    visible: ktxImage.height <= 0
                    width:100; height: 100
                    color: "green"
                }

                //astcImage
                Text {
                    width: 400
                    wrapMode: Text.Wrap
                    text: "An Image element using the shared buffer provider to load an ASTC compressed texture." +
                          "<br>Source: '" + astcImage.source + "'" +
                          (astcImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                }

                Image {
                    id: astcImage
                    source: "image://wlshared/qt4.astc"
                    fillMode: Image.PreserveAspectFit
                }
                Rectangle {
                    visible: astcImage.height <= 0
                    width:100; height: 100
                    color: "green"
                }

                // dynamicImage
                Column {
                    Text {
                        width: 400
                        wrapMode: Text.Wrap
                        text: "An Image element using the shared buffer provider." +
                              "<br>Source: '" + dynamicImage.source + "'" +
                              (dynamicImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                    }
                    Row {
                        spacing: 10
                        Text {
                            text: "Enter filename:"
                        }
                        Rectangle {
                            color: "white"
                            width: sourceEdit.contentWidth + 30
                            height: sourceEdit.contentHeight
                            TextInput {
                                id: sourceEdit
                                anchors.fill: parent
                                horizontalAlignment: TextInput.AlignHCenter
                                onEditingFinished: dynamicImage.source = text ? "image://wlshared/" + text : ""
                            }
                        }
                    }
                }
                Image {
                    id: dynamicImage
                    fillMode: Image.PreserveAspectFit
                }
                Rectangle {
                    visible: dynamicImage.height <= 0
                    width:100; height: 100
                    color: "green"
                }

                // largeImage
                Text {
                    width: 400
                    wrapMode: Text.Wrap
                    text: "An Image element using the shared buffer provider.<br>" +
                          "Left click to load a very large image. " +
                          "Right click to unload the image, potentially freeing graphics memory on the server-side " +
                           "if no other client is using the image." +
                           "<br>Source: '" + largeImage.source + "'" +
                           "<br>Size: " + largeImage.sourceSize +
                           (largeImage.sourceSize.height <= 0 ? "<font color=\"#FF0000\"><br>[Image not loaded]</font>" : "")
                }

                Rectangle {
                    width: 200
                    height: 200
                    border.color: "black"
                    border.width: 2
                    color: "transparent"
                    Image {
                        id: largeImage
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if (mouse.button == Qt.LeftButton)
                                largeImage.source = "image://wlshared/unreasonably large image"
                            else
                                largeImage.source = ""

                        }
                    }
                }

            } // Grid
        }

        Rectangle {
            color: "gray"
            width: parent.width
            height: 20
            anchors.bottom: parent.bottom

            Text {
                color: "white"
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: "Scroll or drag for more"
            }
        }

    }
}
