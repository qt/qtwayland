// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.IviApplication
import QtQuick.Window

WaylandCompositor {
    //! [wayland output]
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true
            Rectangle {
                id: leftArea
                width: parent.width / 2
                height: parent.height
                anchors.left: parent.left
                color: "cornflowerblue"
                Text {
                    anchors.centerIn: parent
                    text: "Ivi surface with id 1337"
                }
            }
            Rectangle {
                id: rightArea
                width: parent.width / 2
                height: parent.height
                anchors.right: parent.right
                color: "burlywood"
                Text {
                    anchors.centerIn: parent
                    text: "Other surfaces"
                }
            }
        }
    }
    //! [wayland output]
    Component {
        id: chromeComponent
        ShellSurfaceItem {
            anchors.fill: parent
            onSurfaceDestroyed: destroy()
            //! [resizing]
            onWidthChanged: handleResized()
            onHeightChanged: handleResized()
            function handleResized() {
                if (width > 0 && height > 0)
                    shellSurface.sendConfigure(Qt.size(width, height));
            }
            //! [resizing]
        }
    }
    //! [connecting]
    IviApplication {
        onIviSurfaceCreated: (iviSurface) =>  {
            var surfaceArea = iviSurface.iviId === 1337 ? leftArea : rightArea;
            var item = chromeComponent.createObject(surfaceArea, { "shellSurface": iviSurface } );
            item.handleResized();
        }
    }
    //! [connecting]
}
