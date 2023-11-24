// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandcompositorwlshell_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlmodule QtWayland.Compositor.WlShell
    \title Qt Wayland WlShell extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the WlShell extension.

    \section2 Summary
    WlShell is a shell extension providing window system features typical to
    desktop systems. It is superseded by XdgShell and exists in Qt mainly
    for backwards compatibility with older applications.

    WlShell corresponds to the Wayland interface \c wl_shell.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.WlShell
    \endqml
*/

QT_END_NAMESPACE

#include "moc_qwaylandcompositorwlshell_p.cpp"
