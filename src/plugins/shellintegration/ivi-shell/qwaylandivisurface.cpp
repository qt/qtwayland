/****************************************************************************
**
** Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qwaylandivisurface_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandextendedsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandIviSurface::QWaylandIviSurface(struct ::ivi_surface *ivi_surface, QWaylandWindow *window)
    : QtWayland::ivi_surface(ivi_surface)
    , QWaylandShellSurface(window)
    , m_window(window)
{
    createExtendedSurface(window);
}

QWaylandIviSurface::QWaylandIviSurface(struct ::ivi_surface *ivi_surface, QWaylandWindow *window,
                                       struct ::ivi_controller_surface *iviControllerSurface)
    : QtWayland::ivi_surface(ivi_surface)
    , QWaylandShellSurface(window)
    , QtWayland::ivi_controller_surface(iviControllerSurface)
    , m_window(window)
{
    createExtendedSurface(window);
}

QWaylandIviSurface::~QWaylandIviSurface()
{
    ivi_surface::destroy();
    if (QtWayland::ivi_controller_surface::object())
        QtWayland::ivi_controller_surface::destroy(0);

    delete m_extendedWindow;
}

void QWaylandIviSurface::applyConfigure()
{
    m_window->resizeFromApplyConfigure(m_pendingSize);
}

void QWaylandIviSurface::createExtendedSurface(QWaylandWindow *window)
{
    if (window->display()->windowExtension())
        m_extendedWindow = new QWaylandExtendedSurface(window);
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
