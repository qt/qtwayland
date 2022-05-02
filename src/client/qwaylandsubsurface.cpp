/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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
******************************************************************************/

#include "qwaylandsubsurface_p.h"

#include "qwaylandwindow_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandSubSurface::QWaylandSubSurface(QWaylandWindow *window, QWaylandWindow *parent, ::wl_subsurface *sub_surface)
    : QtWayland::wl_subsurface(sub_surface)
    , m_window(window)
    , m_parent(parent)
{
    m_parent->mChildren << this;
    setDeSync();
}

QWaylandSubSurface::~QWaylandSubSurface()
{
    m_parent->mChildren.removeOne(this);
    destroy();
}

void QWaylandSubSurface::setSync()
{
    QMutexLocker l(&m_syncLock);
    QWaylandSubSurface::set_sync();
}

void QWaylandSubSurface::setDeSync()
{
    QMutexLocker l(&m_syncLock);
    QWaylandSubSurface::set_desync();
}

void QWaylandSubSurface::set_sync()
{
    m_synchronized = true;
    QtWayland::wl_subsurface::set_sync();
}

void QWaylandSubSurface::set_desync()
{
    m_synchronized = false;
    QtWayland::wl_subsurface::set_desync();
}

}

QT_END_NAMESPACE
