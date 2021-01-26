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

#ifndef QWAYLANDIVISURFACE_H
#define QWAYLANDIVISURFACE_H

#include <QtWaylandCompositor/QWaylandShellSurface>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandIviSurfacePrivate;
class QWaylandSurface;
class QWaylandIviApplication;
class QWaylandSurfaceRole;
class QWaylandResource;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandIviSurface : public QWaylandShellSurfaceTemplate<QWaylandIviSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandIviSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandIviSurface)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(uint iviId READ iviId NOTIFY iviIdChanged)

public:
    QWaylandIviSurface();
    QWaylandIviSurface(QWaylandIviApplication *application, QWaylandSurface *surface, uint iviId, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandIviApplication *iviApplication, QWaylandSurface *surface,
                                uint iviId, const QWaylandResource &resource);

    QWaylandSurface *surface() const;
    uint iviId() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();
    static QWaylandIviSurface *fromResource(::wl_resource *resource);

    Q_INVOKABLE void sendConfigure(const QSize &size);

#if QT_CONFIG(wayland_compositor_quick)
    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;
#endif

Q_SIGNALS:
    void surfaceChanged();
    void iviIdChanged();

private:
    void initialize() override;
};

QT_END_NAMESPACE

#endif // QWAYLANDIVISURFACE_H
