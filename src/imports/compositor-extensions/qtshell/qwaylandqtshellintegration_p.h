// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTSHELLINTEGRATION_H
#define QWAYLANDQTSHELLINTEGRATION_H

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>

#include "qwaylandqtshell.h"

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

class QtShellIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    QtShellIntegration(QWaylandQuickShellSurfaceItem *item);
    ~QtShellIntegration() override;

private Q_SLOTS:
    void handleQtShellSurfaceDestroyed();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    QWaylandQtShellSurface *m_shellSurface = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDQTSHELLINTEGRATION_H
