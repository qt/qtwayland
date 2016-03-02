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

/*!
 * \class QWaylandBufferRef
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief A class which holds a reference to a surface buffer
 *
 * This class can be used to reference a surface buffer. As long as a reference
 * to the buffer exists, it is owned by the compositor and the client cannot modify it.
 */

/*!
 * Constructs a null buffer ref.
 */
QWaylandBufferRef::QWaylandBufferRef()
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = 0;
}

/*!
 * Constructs a reference to \a buffer.
 */
QWaylandBufferRef::QWaylandBufferRef(QtWayland::SurfaceBuffer *buffer)
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = buffer;
    if (buffer)
        buffer->ref();
}

/*!
 * Creates a new reference to the buffer referenced by \a ref.
 */
QWaylandBufferRef::QWaylandBufferRef(const QWaylandBufferRef &ref)
                 : d(new QWaylandBufferRefPrivate)
{
    d->buffer = ref.d->buffer;
    if (d->buffer)
        d->buffer->ref();
}

/*!
 * Dereferences the buffer.
 */
QWaylandBufferRef::~QWaylandBufferRef()
{
    if (d->buffer)
        d->buffer->deref();
    delete d;
}

/*!
 * Assigns \a ref to this buffer. The previously referenced buffer is
 * dereferenced and the new one gets a new reference.
 */
QWaylandBufferRef &QWaylandBufferRef::operator=(const QWaylandBufferRef &ref)
{
    if (d->buffer)
        d->buffer->deref();

    d->buffer = ref.d->buffer;
    if (d->buffer)
        d->buffer->ref();

    return *this;
}

/*!
 * Returns true if this QWaylandBufferRef references the same buffer as \a ref.
 * Otherwise returns false.
 */
bool QWaylandBufferRef::operator==(const QWaylandBufferRef &ref)
{
    return d->buffer == ref.d->buffer;
}

/*!
 * Returns false if this QWaylandBufferRef references the same buffer as \a ref.
 * Otherwise returns true.
 */
bool QWaylandBufferRef::operator!=(const QWaylandBufferRef &ref)
{
    return d->buffer != ref.d->buffer;
}

/*!
 * Returns true if this QWaylandBufferRef does not reference a buffer.
 * Otherwise returns false.
 *
 * \sa hasBuffer()
 */
bool QWaylandBufferRef::isNull() const
{
    return !d->buffer;
}

/*!
 * Returns true if this QWaylandBufferRef references a buffer. Otherwise returns false.
 *
 * \sa isNull()
 */
bool QWaylandBufferRef::hasBuffer() const
{
    return d->buffer;
}

/*!
 * Returns true if this QWaylandBufferRef references a buffer that
 * has been destroyed. Otherwise returns false.
 */
bool QWaylandBufferRef::isDestroyed() const
{
    return d->buffer && d->buffer->isDestroyed();
}

/*!
 * Returns the Wayland resource for the buffer.
 */
struct ::wl_resource *QWaylandBufferRef::wl_buffer() const
{
    return d->buffer ? d->buffer->waylandBufferHandle() : Q_NULLPTR;
}

/*!
 * Returns the size of the buffer.
 * If the buffer referenced is null, an invalid QSize() is returned.
 */
QSize QWaylandBufferRef::size() const
{
    if (d->nullOrDestroyed())
        return QSize();

    return d->buffer->size();
}

/*!
 * Returns the origin of the buffer.
 * If the buffer referenced is null, QWaylandSurface::OriginBottomLeft
 * is returned.
 */
QWaylandSurface::Origin QWaylandBufferRef::origin() const
{
    if (d->buffer)
        return d->buffer->origin();

    return QWaylandSurface::OriginBottomLeft;
}

QWaylandBufferRef::BufferType QWaylandBufferRef::bufferType() const
{
    if (d->nullOrDestroyed())
        return BufferType_Null;

    if (isShm())
        return BufferType_Shm;

    return BufferType_Egl;
}

QWaylandBufferRef::BufferFormatEgl QWaylandBufferRef::bufferFormatEgl() const
{
    if (d->nullOrDestroyed())
        return BufferFormatEgl_Null;

    return d->buffer->bufferFormatEgl();
}

/*!
 * Returns true if the buffer is a shared memory buffer. Otherwise returns false.
 */
bool QWaylandBufferRef::isShm() const
{
    if (d->nullOrDestroyed())
        return false;

    return d->buffer->isShm();
}

/*!
 * Returns an image with the contents of the buffer.
 */
QImage QWaylandBufferRef::image() const
{
    if (d->nullOrDestroyed())
        return QImage();

    return d->buffer->image();
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint QWaylandBufferRef::textureForPlane(int plane) const
{
    if (d->nullOrDestroyed())
        return 0;

    return d->buffer->textureForPlane(plane);
}
#endif

/*!
 * Binds the buffer to the current OpenGL texture. This may
 * perform a copy of the buffer data, depending on the platform
 * and the type of the buffer.
 */
void QWaylandBufferRef::bindToTexture() const
{
    if (d->nullOrDestroyed())
        return;

    return d->buffer->bindToTexture();

}

void QWaylandBufferRef::updateTexture() const
{
    if (d->nullOrDestroyed() || d->buffer->isShm())
        return;

    d->buffer->updateTexture();
}

QT_END_NAMESPACE
