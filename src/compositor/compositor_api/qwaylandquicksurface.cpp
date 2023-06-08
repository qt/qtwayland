// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QSGTexture>
#include <QQuickWindow>
#include <QDebug>

#include "qwaylandquicksurface.h"
#include "qwaylandquicksurface_p.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandquickitem.h"
#include <QtWaylandCompositor/qwaylandbufferref.h>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

QWaylandQuickSurface::QWaylandQuickSurface()
    : QWaylandSurface(* new QWaylandQuickSurfacePrivate())
{

}
QWaylandQuickSurface::QWaylandQuickSurface(QWaylandCompositor *compositor, QWaylandClient *client, quint32 id, int version)
                    : QWaylandSurface(* new QWaylandQuickSurfacePrivate())
{
    initialize(compositor, client, id, version);
}

QWaylandQuickSurface::QWaylandQuickSurface(QWaylandQuickSurfacePrivate &dptr)
    : QWaylandSurface(dptr)
{
}

QWaylandQuickSurface::~QWaylandQuickSurface()
{

}

/*!
 * \qmlproperty bool QtWayland.Compositor::WaylandSurface::useTextureAlpha
 *
 * This property specifies whether the surface should use texture alpha.
 */
bool QWaylandQuickSurface::useTextureAlpha() const
{
    Q_D(const QWaylandQuickSurface);
    return d->useTextureAlpha;
}

void QWaylandQuickSurface::setUseTextureAlpha(bool useTextureAlpha)
{
    Q_D(QWaylandQuickSurface);
    if (d->useTextureAlpha != useTextureAlpha) {
        d->useTextureAlpha = useTextureAlpha;
        emit useTextureAlphaChanged();
        emit configure(d->bufferRef.hasBuffer());
    }
}

QT_END_NAMESPACE

#include "moc_qwaylandquicksurface.cpp"
