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
******************************************************************************/

#ifndef QWAYLANDBUFFERREF_H
#define QWAYLANDBUFFERREF_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QImage>

#if QT_CONFIG(opengl)
#include <QtGui/qopengl.h>
#endif

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QOpenGLTexture;

namespace QtWayland
{
    class ClientBuffer;
}

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandBufferRef
{
public:
    QWaylandBufferRef();
    QWaylandBufferRef(const QWaylandBufferRef &ref);
    ~QWaylandBufferRef();

    QWaylandBufferRef &operator=(const QWaylandBufferRef &ref);
    bool isNull() const;
    bool hasBuffer() const;
    bool hasContent() const;
    bool hasProtectedContent() const;
    bool isDestroyed() const;
    bool operator==(const QWaylandBufferRef &ref);
    bool operator!=(const QWaylandBufferRef &ref);

    struct wl_resource *wl_buffer() const;

    QSize size() const;
    QWaylandSurface::Origin origin() const;

    enum BufferType {
        BufferType_Null,
        BufferType_SharedMemory,
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

    bool isSharedMemory() const;
    QImage image() const;

#if QT_CONFIG(opengl)
    QOpenGLTexture *toOpenGLTexture(int plane = 0) const;
#endif

    quintptr lockNativeBuffer();
    void unlockNativeBuffer(quintptr handle);

private:
    explicit QWaylandBufferRef(QtWayland::ClientBuffer *buffer);
    QtWayland::ClientBuffer *buffer() const;
    class QWaylandBufferRefPrivate *const d;
    friend class QWaylandBufferRefPrivate;
    friend class QWaylandSurfacePrivate;
};

QT_END_NAMESPACE

#endif
