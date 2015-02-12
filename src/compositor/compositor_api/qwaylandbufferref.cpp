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

#include <QDebug>
#include <QAtomicInt>

#include "qwaylandbufferref.h"
#include "wayland_wrapper/qwlsurfacebuffer_p.h"

QT_BEGIN_NAMESPACE

class QWaylandBufferRefPrivate
{
public:
    QtWayland::SurfaceBuffer *buffer;
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
    d->buffer = 0;
    *this = ref;
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

QWaylandBufferRef::operator bool() const
{
    return d->buffer && d->buffer->waylandBufferHandle();
}

bool QWaylandBufferRef::isShm() const
{
    return d->buffer->isShmBuffer();
}

QImage QWaylandBufferRef::image() const
{
    if (d->buffer->isShmBuffer())
        return d->buffer->image();
    return QImage();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL

GLuint QWaylandBufferRef::createTexture()
{
    if (!d->buffer->isShmBuffer() && !d->buffer->textureCreated()) {
        d->buffer->createTexture();
    }
    return d->buffer->texture();
}

void QWaylandBufferRef::destroyTexture()
{
    if (!d->buffer->isShmBuffer() && d->buffer->textureCreated()) {
        d->buffer->destroyTexture();
    }
}

void *QWaylandBufferRef::nativeBuffer() const
{
    return d->buffer->handle();
}
#endif

QT_END_NAMESPACE
