// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLCLIENTBUFFER_P_H
#define QWLCLIENTBUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QRect>
#include <QtGui/qopengl.h>
#include <QImage>
#include <QAtomicInt>
#include <QScopedPointer>

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandBufferRef>
#include <QtCore/private/qglobal_p.h>

#include <wayland-server-core.h>

QT_BEGIN_NAMESPACE

class QWaylandClientBufferIntegration;
class QWaylandBufferRef;
class QWaylandCompositor;
class QOpenGLTexture;

namespace QtWayland {

struct surface_buffer_destroy_listener
{
    struct wl_listener listener;
    class ClientBuffer *surfaceBuffer = nullptr;
};

class Q_WAYLANDCOMPOSITOR_EXPORT ClientBuffer
{
public:
    ClientBuffer(struct ::wl_resource *bufferResource);

    virtual ~ClientBuffer();

    virtual QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const;
    virtual QSize size() const = 0;
    virtual QWaylandSurface::Origin origin() const = 0;

    virtual quintptr lockNativeBuffer() { return 0; }
    virtual void unlockNativeBuffer(quintptr native_buffer) const { Q_UNUSED(native_buffer); }

    virtual QImage image() const { return QImage(); }

    inline bool isCommitted() const { return m_committed; }
    virtual void setCommitted(QRegion &damage);
    bool isDestroyed() { return m_destroyed; }

    virtual bool isProtected() { return false; }

    inline struct ::wl_resource *waylandBufferHandle() const { return m_buffer; }

    bool isSharedMemory() const { return wl_shm_buffer_get(m_buffer); }

#if QT_CONFIG(opengl)
    virtual QOpenGLTexture *toOpenGlTexture(int plane = 0) = 0;
#endif

    static bool hasContent(ClientBuffer *buffer) { return buffer && buffer->waylandBufferHandle(); }
    static bool hasProtectedContent(ClientBuffer *buffer) { return buffer && buffer->isProtected(); }

protected:
    void ref();
    void deref();
    void sendRelease();
    virtual void setDestroyed();

    struct ::wl_resource *m_buffer = nullptr;
    QRegion m_damage;
    bool m_textureDirty = false;

private:
    bool m_committed = false;
    bool m_destroyed = false;

    QAtomicInt m_refCount;

    friend class ::QWaylandBufferRef;
    friend class BufferManager;
};

class Q_WAYLANDCOMPOSITOR_EXPORT SharedMemoryBuffer : public ClientBuffer
{
public:
    SharedMemoryBuffer(struct ::wl_resource *bufferResource);

    QSize size() const override;
    QWaylandSurface::Origin origin() const  override;
    QImage image() const override;

#if QT_CONFIG(opengl)
    QOpenGLTexture *toOpenGlTexture(int plane = 0) override;

private:
    QScopedPointer<QOpenGLTexture> m_shmTexture;
#endif
};

}

QT_END_NAMESPACE

#endif // QWLCLIENTBUFFER_P_H
