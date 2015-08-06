/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandview.h"
#include "qwaylandview_p.h"
#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"
#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/private/qwaylandoutputspace_p.h>
#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

QWaylandViewPrivate *QWaylandViewPrivate::get(QWaylandView *view)
{
    return view->d_func();
}

void QWaylandViewPrivate::markSurfaceAsDestroyed(QWaylandSurface *surface)
{
    Q_Q(QWaylandView);
    Q_ASSERT(surface == this->surface);

    q->setSurface(Q_NULLPTR);
    q->waylandSurfaceDestroyed();
}

QWaylandView::QWaylandView()
                   : d_ptr(new QWaylandViewPrivate(this))
{
}

QWaylandView::~QWaylandView()
{
    Q_D(QWaylandView);
    if (d->output)
        d->output->handle()->removeView(this);
    if (d->surface) {
        QWaylandInputDevice *i = d->surface->compositor()->defaultInputDevice();
        if (i->mouseFocus() == this)
            i->setMouseFocus(Q_NULLPTR);

        d->surface->deref();
    }

}

QWaylandSurface *QWaylandView::surface() const
{
    Q_D(const QWaylandView);
    return d->surface;
}

void QWaylandView::setSurface(QWaylandSurface *newSurface)
{
    Q_D(QWaylandView);
    if (d->surface == newSurface)
        return;

    if (!d->output && newSurface && !d->surface)
        setOutput(newSurface->primaryOutput());

    QWaylandSurface *oldSurface = d->surface;
    d->surface = newSurface;

    if (oldSurface)
        QWaylandSurfacePrivate::get(oldSurface)->derefView(this);

    if (newSurface)
        QWaylandSurfacePrivate::get(newSurface)->refView(this);

    waylandSurfaceChanged(newSurface, oldSurface);
    if (!d->lockedBuffer) {
        d->currentBuffer = QWaylandBufferRef();
        d->currentDamage = QRegion();
    }

    d->nextBuffer = QWaylandBufferRef();
    d->nextDamage = QRegion();
}

QWaylandOutput *QWaylandView::output() const
{
    Q_D(const QWaylandView);
    return d->output;
}

void QWaylandView::setOutput(QWaylandOutput *newOutput)
{
    Q_D(QWaylandView);
    if (d->output == newOutput)
        return;

    QWaylandOutput *oldOutput = d->output;
    d->output = newOutput;

    waylandOutputChanged(newOutput, oldOutput);
}

QWaylandCompositor *QWaylandView::compositor() const
{
    Q_D(const QWaylandView);
    return d->surface ? d->surface->compositor() : 0;
}

void QWaylandView::setRequestedPosition(const QPointF &pos)
{
    Q_D(QWaylandView);
    d->requestedPos = pos;
    if (d->shouldBroadcastRequestedPositionChanged()) {
        Q_ASSERT(d->output->outputSpace());
        QWaylandOutputSpacePrivate *outputSpacePriv = QWaylandOutputSpacePrivate::get(d->output->outputSpace());
        outputSpacePriv->emitSurfacePositionChanged(d->surface, pos);
    }
}

QPointF QWaylandView::requestedPosition() const
{
    Q_D(const QWaylandView);
    return d->requestedPos;
}

QPointF QWaylandView::pos() const
{
    Q_D(const QWaylandView);
    return d->requestedPos;
}

void QWaylandView::attach(const QWaylandBufferRef &ref, const QRegion &damage)
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->nextBuffer = ref;
    d->nextDamage = damage;
}

bool QWaylandView::advance()
{
    Q_D(QWaylandView);
    if (d->currentBuffer == d->nextBuffer)
        return false;

    if (d->lockedBuffer)
        return false;

    QMutexLocker locker(&d->bufferMutex);
    d->currentBuffer = d->nextBuffer;
    d->currentDamage = d->nextDamage;
    return true;
}

QWaylandBufferRef QWaylandView::currentBuffer()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentBuffer;
}

QRegion QWaylandView::currentDamage()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentDamage;
}

bool QWaylandView::lockedBuffer() const
{
    Q_D(const QWaylandView);
    return d->lockedBuffer;
}

void QWaylandView::setLockedBuffer(bool locked)
{
    Q_D(QWaylandView);
    d->lockedBuffer = locked;
}

bool QWaylandView::broadcastRequestedPositionChanged() const
{
    Q_D(const QWaylandView);
    return d->broadcastRequestedPositionChanged;
}

void QWaylandView::setBroadcastRequestedPositionChanged(bool broadcast)
{
    Q_D(QWaylandView);
    d->broadcastRequestedPositionChanged = broadcast;
}

struct wl_resource *QWaylandView::surfaceResource() const
{
    Q_D(const QWaylandView);
    if (!d->surface)
        return Q_NULLPTR;
    return d->surface->resource();
}

void QWaylandView::waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_D(QWaylandView);
    if (d->output)
        d->output->handle()->updateSurfaceForView(this, newSurface, oldSurface);
}

void QWaylandView::waylandSurfaceDestroyed()
{
}

void QWaylandView::waylandOutputChanged(QWaylandOutput *newOutput, QWaylandOutput *oldOutput)
{
    if (oldOutput)
        oldOutput->handle()->removeView(this);

    if (newOutput)
        newOutput->handle()->addView(this);
}

QT_END_NAMESPACE
