// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQml
import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell
import QtQml.Models

WaylandCompositor {
    id: comp

    ListModel {
        id: emulatedScreens
        ListElement { name: "left";   virtualX: 0;    virtualY: 0; width: 800; height: 600 }
        ListElement { name: "middle"; virtualX: 800;  virtualY: 0; width: 800; height: 600 }
        ListElement { name: "right";  virtualX: 1600; virtualY: 0; width: 800; height: 600 }
    }

    property bool emulated: Qt.application.screens.length < 2

    //! [screens]
    Instantiator {
        id: screens
        model: emulated ? emulatedScreens : Qt.application.screens

        delegate: CompositorScreen {
            surfaceArea.color: "lightsteelblue"
            text: name
            compositor: comp
            screen: emulated ? Qt.application.screens[0] : modelData
            Component.onCompleted: if (!comp.defaultOutput) comp.defaultOutput = this
            position: Qt.point(virtualX, virtualY)
            windowed: emulated
        }
    }
    //! [screens]

    Component {
        id: chromeComponent
        Chrome {}
    }

    Component {
        id: moveItemComponent
        Item {}
    }

    Item {
        id: rootItem
    }

    WlShell {
        onWlShellSurfaceCreated: (shellSurface) => handleShellSurfaceCreated(shellSurface)
    }

    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => handleShellSurfaceCreated(xdgSurface)
    }

    function createShellSurfaceItem(shellSurface, moveItem, output) {
        // ![parenting]
        var parentSurfaceItem = output.viewsBySurface[shellSurface.parentSurface];
        var parent = parentSurfaceItem || output.surfaceArea;
        // ![parenting]
        var item = chromeComponent.createObject(parent, {
            "shellSurface": shellSurface,
            "moveItem": moveItem,
            "output": output
        });
        if (parentSurfaceItem) {
            item.x += output.position.x;
            item.y += output.position.y;
        }
        output.viewsBySurface[shellSurface.surface] = item;
    }

    function handleShellSurfaceCreated(shellSurface) {
        var moveItem = moveItemComponent.createObject(rootItem, {
            "x": screens.objectAt(0).position.x,
            "y": screens.objectAt(0).position.y,
            "width": Qt.binding(function() { return shellSurface.surface.width; }),
            "height": Qt.binding(function() { return shellSurface.surface.height; })
        });
        //! [createShellSurfaceItems]
        for (var i = 0; i < screens.count; ++i)
            createShellSurfaceItem(shellSurface, moveItem, screens.objectAt(i));
        //! [createShellSurfaceItems]
    }
}
