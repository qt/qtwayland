/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QQUICKWAYLANDSURFACE_H
#define QQUICKWAYLANDSURFACE_H

#include <QtWaylandCompositor/qwaylandsurface.h>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandQuickSurfacePrivate;
class QWaylandQuickCompositor;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickSurface : public QWaylandSurface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandQuickSurface)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    Q_PROPERTY(bool clientRenderingEnabled READ clientRenderingEnabled WRITE setClientRenderingEnabled NOTIFY clientRenderingEnabledChanged)
public:
    QWaylandQuickSurface();
    QWaylandQuickSurface(QWaylandCompositor *compositor, QWaylandClient *client, quint32 id, int version);
    ~QWaylandQuickSurface() override;

    bool useTextureAlpha() const;
    void setUseTextureAlpha(bool useTextureAlpha);

    Q_DECL_DEPRECATED bool clientRenderingEnabled() const;
    Q_DECL_DEPRECATED void setClientRenderingEnabled(bool enabled);

Q_SIGNALS:
    void useTextureAlphaChanged();
    void clientRenderingEnabledChanged(); //deprecated
};

QT_END_NAMESPACE

#endif
