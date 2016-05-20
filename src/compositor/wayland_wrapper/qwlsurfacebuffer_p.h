/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef SURFACEBUFFER_H
#define SURFACEBUFFER_H

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

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandBufferRef>

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

class QWaylandClientBufferIntegration;
class QWaylandBufferRef;
class QWaylandCompositor;

namespace QtWayland {

struct surface_buffer_destroy_listener
{
    struct wl_listener listener;
    class SurfaceBuffer *surfaceBuffer;
};

class SurfaceBuffer
{
public:
    SurfaceBuffer(QWaylandSurface *surface);

    ~SurfaceBuffer();

    void initialize(struct ::wl_resource *bufferResource);
    void destructBufferState();

    inline bool isRegisteredWithBuffer() const { return m_is_registered_for_buffer; }

    void sendRelease();
    void disown();

    void setDisplayed();

    inline bool isCommitted() const { return m_committed; }
    inline void setCommitted() { m_committed = true; }
    inline bool isDisplayed() const { return m_is_displayed; }

    bool isDestroyed() { return m_destroyed; }

    inline struct ::wl_resource *waylandBufferHandle() const { return m_buffer; }

    void setDestroyIfUnused(bool destroy);

    QSize size() const;
    QWaylandSurface::Origin origin() const;
    bool isShm() const { return wl_shm_buffer_get(m_buffer); }

    QImage image() const;
    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const;
    void bindToTexture() const;
    uint textureForPlane(int plane) const;
    void updateTexture() const;

    static bool hasContent(SurfaceBuffer *buffer) { return buffer && buffer->waylandBufferHandle(); }
private:
    void ref();
    void deref();
    void destroyIfUnused();

    QWaylandSurface *m_surface;
    QWaylandCompositor *m_compositor;
    struct ::wl_resource *m_buffer;
    int m_bufferScale;
    struct surface_buffer_destroy_listener m_destroy_listener;
    bool m_committed;
    bool m_is_registered_for_buffer;
    bool m_surface_has_buffer;
    bool m_destroyed;

    bool m_is_displayed;

    QAtomicInt m_refCount;
    bool m_used;
    bool m_destroyIfUnused;

    static void destroy_listener_callback(wl_listener *listener, void *data);

    friend class ::QWaylandBufferRef;
};

}

QT_END_NAMESPACE

#endif // SURFACEBUFFER_H
