// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDIVISURFACE_H
#define QWAYLANDIVISURFACE_H

#include <QtWaylandCompositor/QWaylandShellSurface>
#if QT_CONFIG(wayland_compositor_quick)
#include <QtWaylandCompositor/qwaylandquickchildren.h>
#endif

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandIviSurfacePrivate;
class QWaylandSurface;
class QWaylandIviApplication;
class QWaylandSurfaceRole;
class QWaylandResource;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandIviSurface : public QWaylandShellSurfaceTemplate<QWaylandIviSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandIviSurface)
#if QT_CONFIG(wayland_compositor_quick)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandIviSurface)
#endif
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(uint iviId READ iviId NOTIFY iviIdChanged)
    Q_MOC_INCLUDE("qwaylandsurface.h")

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
