/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

import QtQuick 2.0
import "compositor.js" as CompositorLogic

Item {
    id: root

    property variant selectedWindow: null
    property bool hasFullscreenWindow: typeof compositor != "undefined" && compositor.fullscreenSurface !== null

    onHasFullscreenWindowChanged: console.log("has fullscreen window: " + hasFullscreenWindow);

    Image {
        id: background
        Behavior on opacity {
            NumberAnimation { easing.type: Easing.InCubic; duration: 400; }
        }
        anchors.fill: parent
        fillMode: Image.Tile
        source: "background.jpg"
        smooth: true
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.selectedWindow = null
            root.focus = true
        }
    }

    MouseArea {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 2
        height: 2
        hoverEnabled: true
        onEntered: {
            root.selectedWindow = null
            root.focus = true
        }
        z: 10
    }

    function windowAdded(window) {
        var windowContainerComponent = Qt.createComponent("WindowContainer.qml");
        var windowContainer = windowContainerComponent.createObject(root);

        window.parent = windowContainer;

        windowContainer.targetWidth = window.width;
        windowContainer.targetHeight = window.height;
        windowContainer.child = window;

        var windowChromeComponent = Qt.createComponent("WindowChrome.qml");
        var windowChrome = windowChromeComponent.createObject(window);

        CompositorLogic.addWindow(windowContainer);

        windowContainer.opacity = 1
        windowContainer.animationsEnabled = true;
        windowContainer.chrome = windowChrome;
    }

    function windowResized(window) {
        var windowContainer = window.parent;
        windowContainer.width = window.width;
        windowContainer.height = window.height;

        CompositorLogic.relayout();
    }

    function windowDestroyed(window) {
        var windowContainer = window.parent;
        if (windowContainer.runDestroyAnimation)
            windowContainer.runDestroyAnimation();
    }

    function removeWindow(window) {
        var windowContainer = window.parent;
        CompositorLogic.removeWindow(windowContainer);
        windowContainer.chrome.destroy();
        windowContainer.destroy();
        compositor.destroyWindow(window);
    }

    onHeightChanged: CompositorLogic.relayout();
    onWidthChanged: CompositorLogic.relayout();
}
