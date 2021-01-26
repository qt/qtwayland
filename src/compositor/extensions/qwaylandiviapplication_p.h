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

#ifndef QWAYLANDIVIAPPLICATION_P_H
#define QWAYLANDIVIAPPLICATION_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-ivi-application.h>

#include <QtWaylandCompositor/QWaylandIviApplication>

#include <QHash>

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

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandIviApplicationPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::ivi_application
{
    Q_DECLARE_PUBLIC(QWaylandIviApplication)

public:
    QWaylandIviApplicationPrivate();
    static QWaylandIviApplicationPrivate *get(QWaylandIviApplication *iviApplication) { return iviApplication->d_func(); }
    void unregisterIviSurface(QWaylandIviSurface *iviSurface);

    QHash<uint, QWaylandIviSurface*> m_iviSurfaces;

protected:
    void ivi_application_surface_create(Resource *resource, uint32_t ivi_id, wl_resource *surface, uint32_t id) override;
};

QT_END_NAMESPACE

#endif // QWAYLANDIVIAPPLICATION_P_H
