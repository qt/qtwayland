// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

WaylandCompositor {
    id: comp

    defaultOutput: shellScreen
    ShellScreen {
        id: shellScreen
        compositor: comp
    }

    GridScreen {
        id: gridScreen
        compositor: comp
    }

    Component {
        id: chromeComponent
        ShellChrome {
        }
    }

    Component {
        id: surfaceComponent
        WaylandSurface {
            id: surface
            signal activated()
            onHasContentChanged: {
                if (hasContent && !cursorSurface) {
                    gridScreen.gridSurfaces.append( { "gridSurface" : surface } );
                } else {
                    for (var i = 0; i < gridScreen.gridSurfaces.count; i++) {
                        if (gridScreen.gridSurfaces.get(i).gridSurface === surface) {
                            gridScreen.gridSurfaces.remove(i,1);
                            break;
                        }
                    }
                }
            }
        }
    }

    XdgOutputManagerV1 {}

    // ![xdgshell]
    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => {
            var item = chromeComponent.createObject(defaultOutput.surfaceArea, { "shellSurface": xdgSurface } );
            item.surface.activated.connect(item.raise);
        }
    }
    // ![xdgshell]

    // ![onSurfaceRequested]
    onSurfaceRequested: (client, id, version) => {
        var surface = surfaceComponent.createObject(comp, { } );
        surface.initialize(comp, client, id, version);
    }
    // ![onSurfaceRequested]
}
