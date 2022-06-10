// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandqtshellintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandSeat>
#include "qwaylandqtshell.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

QtShellIntegration::QtShellIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<QWaylandQtShellSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &QWaylandQtShellSurface::destroyed,
            this, &QtShellIntegration::handleQtShellSurfaceDestroyed);
}

QtShellIntegration::~QtShellIntegration()
{
    m_item->setSurface(nullptr);
}

void QtShellIntegration::handleQtShellSurfaceDestroyed()
{
    m_shellSurface = nullptr;
}

}

QT_END_NAMESPACE

#include "moc_qwaylandqtshellintegration_p.cpp"
