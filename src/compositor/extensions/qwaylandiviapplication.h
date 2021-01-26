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

#ifndef QWAYLANDIVIAPPLICATION_H
#define QWAYLANDIVIAPPLICATION_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandIviSurface;
class QWaylandSurface;
class QWaylandResource;
class QWaylandIviApplicationPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandIviApplication : public QWaylandCompositorExtensionTemplate<QWaylandIviApplication>
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
