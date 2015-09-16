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
#include <QtWaylandCompositor/QWaylandInputDevice>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>
#include <QtWaylandCompositor/private/qwaylandoutput_p.h>

#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

void QWaylandViewPrivate::markSurfaceAsDestroyed(QWaylandSurface *surface)
{
    Q_Q(QWaylandView);
    Q_ASSERT(surface == this->surface);

    q->setSurface(Q_NULLPTR);
    emit q->surfaceDestroyed();
}

QWaylandView::QWaylandView(QObject *renderObject, QObject *parent)
    : QObject(*new QWaylandViewPrivate(),parent)
{
    d_func()->renderObject = renderObject;
}

QWaylandView::~QWaylandView()
{
    Q_D(QWaylandView);
    if (d->surface) {
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);
        QWaylandInputDevice *i = d->surface->compositor()->defaultInputDevice();
        if (i->mouseFocus() == this)
            i->setMouseFocus(Q_NULLPTR);

        QWaylandSurfacePrivate::get(d->surface)->derefView(this);
    }

}

QObject *QWaylandView::renderObject() const
{
    Q_D(const QWaylandView);
    return d->renderObject;
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


    if (d->surface) {
        QWaylandSurfacePrivate::get(d->surface)->derefView(this);
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);
    }

    d->surface = newSurface;

    if (!d->bufferLock) {
        d->currentBuffer = QWaylandBufferRef();
        d->currentDamage = QRegion();
    }

    d->nextBuffer = QWaylandBufferRef();
    d->nextDamage = QRegion();

    if (d->surface) {
        QWaylandSurfacePrivate::get(d->surface)->refView(this);
        if (d->output)
            QWaylandOutputPrivate::get(d->output)->addView(this, d->surface);
    }

    emit surfaceChanged();

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

    if (d->output && d->surface)
        QWaylandOutputPrivate::get(d->output)->removeView(this, d->surface);

    d->output = newOutput;

    if (d->output && d->surface)
        QWaylandOutputPrivate::get(d->output)->addView(this, d->surface);

    emit outputChanged();
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
    if (d->currentBuffer == d->nextBuffer && !d->forceAdvanceSucceed)
        return false;

    if (d->bufferLock)
        return false;

    QMutexLocker locker(&d->bufferMutex);
    d->forceAdvanceSucceed = false;
    d->currentBuffer = d->nextBuffer;
    d->currentDamage = d->nextDamage;
    return true;
}

void QWaylandView::discardCurrentBuffer()
{
    Q_D(QWaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->currentBuffer = QWaylandBufferRef();
    d->forceAdvanceSucceed = true;
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

bool QWaylandView::isBufferLocked() const
{
    Q_D(const QWaylandView);
    return d->bufferLock;
}

void QWaylandView::setBufferLock(bool locked)
{
    Q_D(QWaylandView);
    d->bufferLock = locked;
}

struct wl_resource *QWaylandView::surfaceResource() const
{
    Q_D(const QWaylandView);
    if (!d->surface)
        return Q_NULLPTR;
    return d->surface->resource();
}

QT_END_NAMESPACE
