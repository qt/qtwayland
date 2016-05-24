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

#ifndef QWAYLANDBUFFERREF_H
#define QWAYLANDBUFFERREF_H

#include <QImage>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/qwaylandexport.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

namespace QtWayland
{
    class SurfaceBuffer;
}

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandBufferRef
{
public:
    QWaylandBufferRef();
    explicit QWaylandBufferRef(QtWayland::SurfaceBuffer *buffer);
    QWaylandBufferRef(const QWaylandBufferRef &ref);
    ~QWaylandBufferRef();

    QWaylandBufferRef &operator=(const QWaylandBufferRef &ref);
    bool isNull() const;
    bool hasBuffer() const;
    bool isDestroyed() const;
    bool operator==(const QWaylandBufferRef &ref);
    bool operator!=(const QWaylandBufferRef &ref);

    struct wl_resource *wl_buffer() const;

    QSize size() const;
    QWaylandSurface::Origin origin() const;

    enum BufferType {
        BufferType_Null,
        BufferType_Shm,
        BufferType_Egl
    };

    enum BufferFormatEgl {
        BufferFormatEgl_Null,
        BufferFormatEgl_RGB,
        BufferFormatEgl_RGBA,
        BufferFormatEgl_EXTERNAL_OES,
        BufferFormatEgl_Y_U_V,
        BufferFormatEgl_Y_UV,
        BufferFormatEgl_Y_XUXV
    };

    BufferType bufferType() const;
    BufferFormatEgl bufferFormatEgl() const;

    bool isShm() const;
    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint textureForPlane(int plane) const;
#endif

    void bindToTexture() const;
    void updateTexture() const;

private:
    class QWaylandBufferRefPrivate *const d;
    friend class QWaylandBufferRefPrivate;
};

QT_END_NAMESPACE

#endif
