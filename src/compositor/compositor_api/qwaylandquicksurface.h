// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQUICKWAYLANDSURFACE_H
#define QQUICKWAYLANDSURFACE_H

#include <QtWaylandCompositor/qwaylandsurface.h>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

struct wl_client;

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QWaylandQuickSurfacePrivate;
class QWaylandQuickCompositor;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickSurface : public QWaylandSurface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandQuickSurface)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    QML_NAMED_ELEMENT(WaylandSurface)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandQuickSurface();
    QWaylandQuickSurface(QWaylandCompositor *compositor, QWaylandClient *client, quint32 id, int version);
    ~QWaylandQuickSurface() override;

    bool useTextureAlpha() const;
    void setUseTextureAlpha(bool useTextureAlpha);

protected:
    QWaylandQuickSurface(QWaylandQuickSurfacePrivate &dptr);

Q_SIGNALS:
    void useTextureAlphaChanged();
};

QT_END_NAMESPACE

#endif
