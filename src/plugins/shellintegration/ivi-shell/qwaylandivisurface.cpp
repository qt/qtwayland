// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandivisurface_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandIviSurface::QWaylandIviSurface(struct ::ivi_surface *ivi_surface, QWaylandWindow *window)
    : QtWayland::ivi_surface(ivi_surface)
    , QWaylandShellSurface(window)
    , m_window(window)
{
}

QWaylandIviSurface::QWaylandIviSurface(struct ::ivi_surface *ivi_surface, QWaylandWindow *window,
                                       struct ::ivi_controller_surface *iviControllerSurface)
    : QtWayland::ivi_surface(ivi_surface)
    , QWaylandShellSurface(window)
    , QtWayland::ivi_controller_surface(iviControllerSurface)
    , m_window(window)
{
}

QWaylandIviSurface::~QWaylandIviSurface()
{
    ivi_surface::destroy();
    if (QtWayland::ivi_controller_surface::object())
        QtWayland::ivi_controller_surface::destroy(0);
}

void QWaylandIviSurface::applyConfigure()
{
    m_window->resizeFromApplyConfigure(m_pendingSize);
}

void QWaylandIviSurface::ivi_surface_configure(int32_t width, int32_t height)
{
    m_pendingSize = {width, height};
    m_window->applyConfigureWhenPossible();
}

void QWaylandIviSurface::ivi_controller_surface_visibility(int32_t visibility)
{
    this->m_window->window()->setVisible(visibility != 0);
}

}

QT_END_NAMESPACE
