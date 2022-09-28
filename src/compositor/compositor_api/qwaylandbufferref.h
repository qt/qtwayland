// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDBUFFERREF_H
#define QWAYLANDBUFFERREF_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtGui/QImage>

#if QT_CONFIG(opengl)
#include <QtGui/qopengl.h>
#endif

#include <QtWaylandCompositor/QWaylandSurface>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QOpenGLTexture;

namespace QtWayland
{
    class ClientBuffer;
}

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandBufferRef
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
#if QT_WAYLANDCOMPOSITOR_REMOVED_SINCE(6, 3)
    bool operator==(const QWaylandBufferRef &ref);
    bool operator!=(const QWaylandBufferRef &ref);
#endif

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

    friend Q_WAYLANDCOMPOSITOR_EXPORT
    bool operator==(const QWaylandBufferRef &lhs, const QWaylandBufferRef &rhs) noexcept;
    friend inline
    bool operator!=(const QWaylandBufferRef &lhs, const QWaylandBufferRef &rhs) noexcept
    { return !(lhs == rhs); }
};

QT_END_NAMESPACE

#endif
