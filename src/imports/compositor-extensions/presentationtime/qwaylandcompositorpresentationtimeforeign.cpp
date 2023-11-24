// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandcompositorpresentationtimeforeign_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlmodule QtWayland.Compositor.PresentationTime
    \title Qt Wayland Presentation Time Extension
    \ingroup qmlmodules
    \since 6.3
    \brief Provides tracking the timing when a frame is presented on screen.

    \section2 Summary
    The PresentationTime extension provides a way to track rendering timing
    for a surface. Client can request feedbacks associated with a surface,
    then compositor send events for the feedback with the time when the surface
    is presented on-screen.

    PresentationTime corresponds to the Wayland \c wp_presentation interface.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.PresentationTime
    \endqml
*/

QT_END_NAMESPACE

#include "moc_qwaylandcompositorpresentationtimeforeign_p.cpp"
