/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QSGTexture>
#include <QOpenGLTexture>
#include <QQuickWindow>
#include <QDebug>
#include <QQmlPropertyMap>

#include "qwaylandquicksurface.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandsurfaceitem.h"
#include "qwaylandoutput.h"
#include <QtCompositor/qwaylandbufferref.h>
#include <QtCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

class BufferAttacher : public QWaylandBufferAttacher
{
public:
    BufferAttacher()
        : surface(0)
        , texture(0)
        , update(false)
    {

    }

    ~BufferAttacher()
    {
        if (texture)
            texture->deleteLater();
        bufferRef = QWaylandBufferRef();
        nextBuffer = QWaylandBufferRef();
    }

    void attach(const QWaylandBufferRef &ref) Q_DECL_OVERRIDE
    {
        nextBuffer = ref;
        update = true;
    }

    void createTexture()
    {
        if (bufferRef)
            bufferRef.destroyTexture();
        bufferRef = nextBuffer;

        QQuickWindow *window = static_cast<QQuickWindow *>(surface->mainOutput()->window());

        // If the next buffer is NULL do not delete the current texture. If the client called
        // attach(0) the surface is going to be unmapped anyway, if instead the client attached
        // a valid buffer but died before we got here we want to keep the old buffer around
        // in case some destroy animation is run.
        if (bufferRef) {
            delete texture;

            if (bufferRef.isShm()) {
                texture = window->createTextureFromImage(bufferRef.image());
            } else {
                QQuickWindow::CreateTextureOptions opt = 0;
                if (surface->useTextureAlpha()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }
                texture = window->createTextureFromId(bufferRef.createTexture(), surface->size(), opt);
            }
            texture->bind();
        }

        update = false;
    }

    void invalidateTexture()
    {
        if (bufferRef)
            bufferRef.destroyTexture();
        delete texture;
        texture = 0;
        update = true;
        bufferRef = QWaylandBufferRef();
    }

    QWaylandQuickSurface *surface;
    QWaylandBufferRef bufferRef;
    QWaylandBufferRef nextBuffer;
    QSGTexture *texture;
    bool update;
};


class QWaylandQuickSurfacePrivate : public QWaylandSurfacePrivate
{
public:
    QWaylandQuickSurfacePrivate(wl_client *client, quint32 id, int version, QWaylandQuickCompositor *c, QWaylandQuickSurface *surf)
        : QWaylandSurfacePrivate(client, id, version, c, surf)
        , buffer(new BufferAttacher)
        , compositor(c)
        , useTextureAlpha(true)
        , windowPropertyMap(new QQmlPropertyMap)
        , clientRenderingEnabled(true)
    {

    }

    ~QWaylandQuickSurfacePrivate()
    {
        windowPropertyMap->deleteLater();
        // buffer is deleted automatically by ~Surface(), since it is the assigned attacher
    }

    void surface_commit(Resource *resource) Q_DECL_OVERRIDE
    {
        if (m_pending.newlyAttached) {
            buffer->update = true;
        }
        QWaylandSurfacePrivate::surface_commit(resource);

        Q_FOREACH (QtWayland::Output *output, outputs())
            output->waylandOutput()->update();
    }

    BufferAttacher *buffer;
    QWaylandQuickCompositor *compositor;
    bool useTextureAlpha;
    QQmlPropertyMap *windowPropertyMap;
    bool clientRenderingEnabled;
};

QWaylandQuickSurface::QWaylandQuickSurface(wl_client *client, quint32 id, int version, QWaylandQuickCompositor *compositor)
                    : QWaylandSurface(new QWaylandQuickSurfacePrivate(client, id, version, compositor, this))
{
    Q_D(QWaylandQuickSurface);
    d->buffer->surface = this;
    setBufferAttacher(d->buffer);

    connect(this, &QWaylandSurface::windowPropertyChanged, d->windowPropertyMap, &QQmlPropertyMap::insert);
    connect(d->windowPropertyMap, &QQmlPropertyMap::valueChanged, this, &QWaylandSurface::setWindowProperty);
}

QWaylandQuickSurface::~QWaylandQuickSurface()
{

}

QSGTexture *QWaylandQuickSurface::texture() const
{
    Q_D(const QWaylandQuickSurface);
    return d->buffer->texture;
}

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
        emit configure(d->buffer->bufferRef);
    }
}

QObject *QWaylandQuickSurface::windowPropertyMap() const
{
    Q_D(const QWaylandQuickSurface);
    return d->windowPropertyMap;
}

bool QWaylandQuickSurface::event(QEvent *e)
{
    if (e->type() == static_cast<QEvent::Type>(QWaylandSurfaceLeaveEvent::WaylandSurfaceLeave)) {
        QWaylandSurfaceLeaveEvent *event = static_cast<QWaylandSurfaceLeaveEvent *>(e);

        if (event->output()) {
            QQuickWindow *oldWindow = static_cast<QQuickWindow *>(event->output()->window());
            disconnect(oldWindow, &QQuickWindow::beforeSynchronizing,
                       this, &QWaylandQuickSurface::updateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphInvalidated,
                       this, &QWaylandQuickSurface::invalidateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphAboutToStop,
                       this, &QWaylandQuickSurface::invalidateTexture);
        }

        return true;
    }

    if (e->type() == static_cast<QEvent::Type>(QWaylandSurfaceEnterEvent::WaylandSurfaceEnter)) {
        QWaylandSurfaceEnterEvent *event = static_cast<QWaylandSurfaceEnterEvent *>(e);

        if (event->output()) {
            QQuickWindow *window = static_cast<QQuickWindow *>(event->output()->window());
            connect(window, &QQuickWindow::beforeSynchronizing,
                    this, &QWaylandQuickSurface::updateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphInvalidated,
                    this, &QWaylandQuickSurface::invalidateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphAboutToStop,
                    this, &QWaylandQuickSurface::invalidateTexture,
                    Qt::DirectConnection);
        }

        return true;
    }

    return QObject::event(e);
}

void QWaylandQuickSurface::updateTexture()
{
    Q_D(QWaylandQuickSurface);
    const bool update = d->buffer->update;
    if (d->buffer->update)
        d->buffer->createTexture();
    foreach (QWaylandSurfaceView *view, views())
        static_cast<QWaylandSurfaceItem *>(view)->updateTexture(update);
}

void QWaylandQuickSurface::invalidateTexture()
{
    Q_D(QWaylandQuickSurface);
    d->buffer->invalidateTexture();
    foreach (QWaylandSurfaceView *view, views())
        static_cast<QWaylandSurfaceItem *>(view)->updateTexture(true);
    emit redraw();
}

bool QWaylandQuickSurface::clientRenderingEnabled() const
{
    Q_D(const QWaylandQuickSurface);
    return d->clientRenderingEnabled;
}

void QWaylandQuickSurface::setClientRenderingEnabled(bool enabled)
{
    Q_D(QWaylandQuickSurface);
    if (d->clientRenderingEnabled != enabled) {
        d->clientRenderingEnabled = enabled;

        sendOnScreenVisibilityChange(enabled);

        emit clientRenderingEnabledChanged();
    }
}

QT_END_NAMESPACE
