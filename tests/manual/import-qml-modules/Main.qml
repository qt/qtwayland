// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtWayland.Compositor.IviApplication
import QtWayland.Compositor.PresentationTime
import QtWayland.Compositor.QtShell
import QtWayland.Compositor.WlShell
import QtWayland.Compositor.XdgShell


Item {
    property var p1: IviApplication {}
    property var p2: PresentationTime {}
    property var p3: QtShellChrome {}
    property var p4: WlShellSurface {}
    property var p5: XdgPopup {}
}
