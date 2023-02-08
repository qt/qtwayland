// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

import io.qt.examples.customextension 1.0

WaylandCompositor {
    id: comp

    property alias customExtension: custom
    property var itemList: []

    function itemForSurface(surface) {
        var n = itemList.length
        for (var i = 0; i < n; i++) {
            if (itemList[i].surface === surface)
                return itemList[i]
        }
    }

    CompositorScreen {
        compositor: comp
    }

    Component {
        id: chromeComponent
        ShellSurfaceItem {
            id: chrome

            property bool isCustom
            property int fontSize: 12

            onSurfaceDestroyed: {
                var index = itemList.indexOf(chrome)
                if (index > -1) {
                    var listCopy = itemList
                    listCopy.splice(index, 1)
                    itemList = listCopy
                }
                chrome.destroy()
            }

            transform: [
                Rotation {
                    id: xRot
                    origin.x: chrome.width / 2; origin.y: chrome.height / 2
                    angle: 0
                    axis { x: 1; y: 0; z: 0 }
                },
                Rotation {
                    id: yRot
                    origin.x: chrome.width / 2; origin.y: chrome.height / 2
                    angle: 0
                    axis { x: 0; y: 1; z: 0 }
                }
            ]

            NumberAnimation {
                id: spinAnimation
                running: false
                loops: 2
                target: yRot
                property: "angle"
                from: 0; to: 360
                duration: 400
            }

            function doSpin(ms) {
                console.log("spin " + ms)
                // using the 'ms' argument is left as an exercise for the reader...
                spinAnimation.start()
            }

            NumberAnimation {
                id: bounceAnimation
                running: false
                target: chrome
                property: "y"
                from: 0
                to: output.window.height - chrome.height
                easing.type: Easing.OutBounce
                duration: 1000
            }

            function doBounce(ms) {
                console.log("bounce " + ms)
                // using the 'ms' argument is left as an exercise for the reader...
                bounceAnimation.start()
            }

//! [setFontSize]
            onFontSizeChanged: {
                custom.setFontSize(surface, fontSize)
            }
//! [setFontSize]
        }
    }

    Component {
        id: customObjectComponent
        Rectangle {
            id: customItem
            property QtObject obj
            property alias text: label.text

            width: 100
            height: 100
            radius: width / 2
            x: Math.random() * (defaultOutput.surfaceArea.width - 100)
            y: Math.random() * (defaultOutput.surfaceArea.height - 100)

            Text {
                id: label
                anchors.centerIn: parent
                text: "?"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: obj.sendClicked()
            }

            Connections {
                target: obj
                function onResourceDestroyed() {
                    customItem.destroy()
                }
            }
        }
    }

    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => {
            var item = chromeComponent.createObject(defaultOutput.surfaceArea, { "shellSurface": xdgSurface } )
            var w = defaultOutput.surfaceArea.width / 2
            var h = defaultOutput.surfaceArea.height / 2
            item.x = Math.random() * w
            item.y = Math.random() * h
            var listCopy = itemList // List properties cannot be modified through Javascript operations
            listCopy.push(item)
            itemList = listCopy
        }
    }

//! [CustomExtension]
    CustomExtension {
        id: custom

        onSurfaceAdded: (surface) => {
            var item = itemForSurface(surface)
            item.isCustom = true
        }

        onBounce: (surface, ms) => {
            var item = itemForSurface(surface)
            item.doBounce(ms)
        }

        onSpin: (surface, ms) => {
            var item = itemForSurface(surface)
            item.doSpin(ms)
        }

        onCustomObjectCreated: (obj) => {
            var item = customObjectComponent.createObject(defaultOutput.surfaceArea,
                                                          { "color": obj.color,
                                                            "text": obj.text,
                                                            "obj": obj } )
        }
    }

    function setDecorations(shown) {
        var n = itemList.length
        for (var i = 0; i < n; i++) {
            if (itemList[i].isCustom)
                custom.showDecorations(itemList[i].surface.client, shown)
        }
    }
//! [CustomExtension]
}
