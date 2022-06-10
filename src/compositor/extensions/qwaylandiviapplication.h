// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDIVIAPPLICATION_H
#define QWAYLANDIVIAPPLICATION_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandIviSurface>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandIviApplicationPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandIviApplication : public QWaylandCompositorExtensionTemplate<QWaylandIviApplication>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandIviApplication)

public:
    QWaylandIviApplication();
    QWaylandIviApplication(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void iviSurfaceRequested(QWaylandSurface *surface, uint iviId, const QWaylandResource &resource);
    void iviSurfaceCreated(QWaylandIviSurface *iviSurface);
};

QT_END_NAMESPACE

#endif // QWAYLANDIVIAPPLICATION_H
