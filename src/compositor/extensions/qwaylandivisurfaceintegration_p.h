/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
****************************************************************************/

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
