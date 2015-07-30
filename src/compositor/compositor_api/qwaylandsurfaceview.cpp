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

#include "qwaylandsurfaceview.h"
#include "qwaylandsurfaceview_p.h"
#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"

#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

QWaylandSurfaceViewPrivate *QWaylandSurfaceViewPrivate::get(QWaylandSurfaceView *view)
{
    return view->d;
}

void QWaylandSurfaceViewPrivate::markSurfaceAsDestroyed(QWaylandSurface *surface)
{
    Q_ASSERT(surface == this->surface);

    q_ptr->waylandSurfaceDestroyed();
    q_ptr->setSurface(Q_NULLPTR);
}

QWaylandSurfaceView::QWaylandSurfaceView()
                   : d(new QWaylandSurfaceViewPrivate(this))
{
}

QWaylandSurfaceView::~QWaylandSurfaceView()
{
    if (d->output)
        d->output->handle()->removeView(this);
    if (d->surface) {
        QWaylandInputDevice *i = d->surface->compositor()->defaultInputDevice();
        if (i->mouseFocus() == this)
            i->setMouseFocus(Q_NULLPTR, QPointF());

        d->surface->deref();
    }

    delete d;
}

QWaylandSurface *QWaylandSurfaceView::surface() const
{
    return d->surface;
}

void QWaylandSurfaceView::setSurface(QWaylandSurface *newSurface)
{
    QWaylandSurface *oldSurface = d->surface;
    d->surface = newSurface;

    if (oldSurface)
        QWaylandSurfacePrivate::get(oldSurface)->derefView(this);

    if (newSurface)
        QWaylandSurfacePrivate::get(newSurface)->refView(this);

    waylandSurfaceChanged(newSurface, oldSurface);
    if (!d->lockedBuffer)
        d->currentBuffer = QWaylandBufferRef();

    d->nextBuffer = QWaylandBufferRef();
}

QWaylandOutput *QWaylandSurfaceView::output() const
{
    return d->output;
}

void QWaylandSurfaceView::setOutput(QWaylandOutput *newOutput)
{
    if (d->output == newOutput)
        return;

    QWaylandOutput *oldOutput = d->output;
    d->output = newOutput;

    waylandOutputChanged(newOutput, oldOutput);
}

QWaylandCompositor *QWaylandSurfaceView::compositor() const
{
    return d->surface ? d->surface->compositor() : 0;
}

void QWaylandSurfaceView::setRequestedPosition(const QPointF &pos)
{
    d->requestedPos = pos;
    if (d->shouldBroadcastRequestedPositionChanged()) {
        Q_ASSERT(d->output->outputSpace());
        QWaylandOutputSpacePrivate *outputSpacePriv = QWaylandOutputSpacePrivate::get(d->output->outputSpace());
        outputSpacePriv->emitSurfacePositionChanged(d->surface, pos);
    }
}

QPointF QWaylandSurfaceView::requestedPosition() const
{
    return d->requestedPos;
}

QPointF QWaylandSurfaceView::pos() const
{
    return d->requestedPos;
}

void QWaylandSurfaceView::attach(const QWaylandBufferRef &ref)
{
    QMutexLocker locker(&d->bufferMutex);
    d->nextBuffer = ref;
}

bool QWaylandSurfaceView::advance()
{
    if (d->currentBuffer == d->nextBuffer)
        return false;

    if (d->lockedBuffer)
        return false;

    QMutexLocker locker(&d->bufferMutex);
    d->currentBuffer = d->nextBuffer;
    return true;
}

QWaylandBufferRef QWaylandSurfaceView::currentBuffer()
{
    QMutexLocker locker(&d->bufferMutex);
    return d->currentBuffer;
}

bool QWaylandSurfaceView::lockedBuffer() const
{
    return d->lockedBuffer;
}

void QWaylandSurfaceView::setLockedBuffer(bool locked)
{
    d->lockedBuffer = locked;
}

bool QWaylandSurfaceView::broadcastRequestedPositionChanged() const
{
    return d->broadcastRequestedPositionChanged;
}

void QWaylandSurfaceView::setBroadcastRequestedPositionChanged(bool broadcast)
{
    d->broadcastRequestedPositionChanged = broadcast;
}

void QWaylandSurfaceView::waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    if (d->output)
        d->output->handle()->updateSurfaceForView(this, newSurface, oldSurface);
}

void QWaylandSurfaceView::waylandSurfaceDestroyed()
{
}

void QWaylandSurfaceView::waylandOutputChanged(QWaylandOutput *newOutput, QWaylandOutput *oldOutput)
{
    if (oldOutput)
        oldOutput->handle()->removeView(this);

    if (newOutput)
        newOutput->handle()->addView(this);
}

QT_END_NAMESPACE
