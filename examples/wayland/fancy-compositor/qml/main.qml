// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell
import QtWayland.Compositor.IviApplication

WaylandCompositor {
    id: waylandCompositor

    CompositorScreen { id: screen; compositor: waylandCompositor }

    // ![shell extensions]
    // Shell surface extension. Needed to provide a window concept for Wayland clients.
    // I.e. requests and events for maximization, minimization, resizing, closing etc.
    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => screen.handleShellSurface(xdgSurface)
    }

    // Minimalistic shell extension. Mainly used for embedded applications.
    IviApplication {
        onIviSurfaceCreated: (iviSurface) => screen.handleShellSurface(iviSurface)
    }

    // Deprecated shell extension, still used by some clients
    WlShell {
        onWlShellSurfaceCreated: (shellSurface) => screen.handleShellSurface(shellSurface)
    }
    // ![shell extensions]

    // Extension for Input Method (QT_IM_MODULE) support at compositor-side
    // ![text input]
    TextInputManager {}
    QtTextInputMethodManager {}
    // ![text input]
}
