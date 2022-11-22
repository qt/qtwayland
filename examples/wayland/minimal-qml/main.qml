// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell
import QtWayland.Compositor.IviApplication

//! [compositor]
WaylandCompositor {
//! [compositor]
    // The output defines the screen.

    //! [output]
    WaylandOutput {
        sizeFollowsWindow: true
        window: Window {
            width: 1024
            height: 768
            visible: true
    //! [output]

            //! [shell surface item]
            Repeater {
                model: shellSurfaces
                // ShellSurfaceItem handles displaying a shell surface.
                // It has implementations for things like interactive
                // resize/move, and forwarding of mouse and keyboard
                // events to the client process.
                ShellSurfaceItem {
                    shellSurface: modelData
                    onSurfaceDestroyed: shellSurfaces.remove(index)
                }
            }
            //! [shell surface item]
        }
    }

    // Extensions are additions to the core Wayland
    // protocol. We choose to support three different
    // shells (window management protocols). When the
    // client creates a new shell surface (i.e. a window)
    // we append it to our list of shellSurfaces.

    //! [shells]
    WlShell {
        onWlShellSurfaceCreated: (shellSurface) => shellSurfaces.append({shellSurface: shellSurface});
    }
    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => shellSurfaces.append({shellSurface: xdgSurface});
    }
    IviApplication {
        onIviSurfaceCreated: (iviSurface) => shellSurfaces.append({shellSurface: iviSurface});
    }
    //! [shells]

    //! [model]
    ListModel { id: shellSurfaces }
    //! [model]
}
