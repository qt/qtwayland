// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDIVISURFACEINTEGRATION_H
#define QWAYLANDIVISURFACEINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>

#include <QtWaylandCompositor/QWaylandIviSurface>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace QtWayland {

class IviSurfaceIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    IviSurfaceIntegration(QWaylandQuickShellSurfaceItem *item);
    ~IviSurfaceIntegration() override;

private Q_SLOTS:
    void handleIviSurfaceDestroyed();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandIviSurface *m_shellSurface = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDIVISURFACEINTEGRATION_H
