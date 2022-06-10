// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandivisurfaceintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandIviSurface>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandSeat>

QT_BEGIN_NAMESPACE

namespace QtWayland {

IviSurfaceIntegration::IviSurfaceIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<QWaylandIviSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &QWaylandIviSurface::destroyed, this, &IviSurfaceIntegration::handleIviSurfaceDestroyed);
}

IviSurfaceIntegration::~IviSurfaceIntegration()
{
    m_item->setSurface(nullptr);
}

void IviSurfaceIntegration::handleIviSurfaceDestroyed()
{
    m_shellSurface = nullptr;
}

}

QT_END_NAMESPACE

#include "moc_qwaylandivisurfaceintegration_p.cpp"
