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

#include <QDebug>
#include <QAtomicInt>

#include "qwaylandbufferref.h"
#include "wayland_wrapper/qwlsurfacebuffer_p.h"

QT_BEGIN_NAMESPACE

class QWaylandBufferRefPrivate
{
public:
    QtWayland::SurfaceBuffer *buffer;

    bool nullOrDestroyed() {
        return !buffer || buffer->isDestroyed();
    }
};

QWaylandBufferRef::QWaylandBufferRef()
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = 0;
}

QWaylandBufferRef::QWaylandBufferRef(QtWayland::SurfaceBuffer *buffer)
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = buffer;
    if (buffer)
        buffer->ref();
}

QWaylandBufferRef::QWaylandBufferRef(const QWaylandBufferRef &ref)
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = ref.d->buffer;
    if (d->buffer)
        d->buffer->ref();
}

QWaylandBufferRef::~QWaylandBufferRef()
{
    if (d->buffer)
        d->buffer->deref();
    delete d;
}

QWaylandBufferRef &QWaylandBufferRef::operator=(const QWaylandBufferRef &ref)
{
    if (d->buffer)
        d->buffer->deref();

    d->buffer = ref.d->buffer;
    if (d->buffer)
        d->buffer->ref();

    return *this;
}

bool QWaylandBufferRef::operator==(const QWaylandBufferRef &ref)
{
    return d->buffer == ref.d->buffer;
}

bool QWaylandBufferRef::operator!=(const QWaylandBufferRef &ref)
{
    return d->buffer != ref.d->buffer;
}

bool QWaylandBufferRef::isNull() const
{
    return !d->buffer;
}

bool QWaylandBufferRef::hasBuffer() const
{
    return d->buffer && !d->buffer->isDestroyed();
}

struct ::wl_resource *QWaylandBufferRef::wl_buffer() const
{
    return d->buffer ? d->buffer->waylandBufferHandle() : Q_NULLPTR;
}

QSize QWaylandBufferRef::size() const
{
    if (d->nullOrDestroyed())
        return QSize();

    return d->buffer->size();
}

QWaylandSurface::Origin QWaylandBufferRef::origin() const
{
    if (d->nullOrDestroyed())
        return QWaylandSurface::OriginBottomLeft;

    return d->buffer->origin();
}

bool QWaylandBufferRef::isShm() const
{
    if (d->nullOrDestroyed())
        return false;

    return d->buffer->isShm();
}

QImage QWaylandBufferRef::image() const
{
    if (d->nullOrDestroyed())
        return QImage();

    return d->buffer->image();
}

void QWaylandBufferRef::bindToTexture() const
{
    if (d->nullOrDestroyed())
        return;

    return d->buffer->bindToTexture();

}

void *QWaylandBufferRef::nativeBuffer() const
{
    return d->buffer->handle();
}

QT_END_NAMESPACE
