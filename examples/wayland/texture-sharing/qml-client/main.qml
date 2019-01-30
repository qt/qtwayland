/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Wayland module
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

import QtQuick 2.9
import QtQuick.Window 2.2

import QtWayland.Client.TextureSharing 1.0

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
