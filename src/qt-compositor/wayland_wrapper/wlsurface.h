/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef WL_SURFACE_H
#define WL_SURFACE_H

#include "waylandobject.h"
#include "wlshmbuffer.h"
#include "waylandsurface.h"

#include <QtCore/QRect>
#include <QtGui/QImage>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/QOpenGLContext>
#include <QtGui/qopengl.h>
#endif

namespace Wayland {

class Compositor;
class Buffer;

class SurfacePrivate;

class Surface : public Object<struct wl_surface>
{
    Q_DECLARE_PRIVATE(Surface)
public:
    Surface(struct wl_client *client, Compositor *compositor);
    ~Surface();

    WaylandSurface::Type type() const;
    bool isYInverted() const;

    uint id() const { return base()->resource.object.id; }
    void attach(struct wl_resource *buffer);

    void damage(const QRect &rect);

    QImage image() const;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint textureId(QOpenGLContext *context) const;
#endif

    void sendMousePressEvent(int x, int y, Qt::MouseButton button);
    void sendMouseReleaseEvent(int x, int y, Qt::MouseButton button);
    void sendMouseMoveEvent(int x, int y);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFrameCallback();

    void frameFinished();
    void setInputFocus();

    void sendOnScreenVisibilityChange(bool visible);

    WaylandSurface *handle() const;
    wl_client *clientHandle() const;
    qint64 processId() const;
    void setProcessId(qint64 processId);
    QByteArray authenticationToken() const;
    void setAuthenticationToken(const QByteArray &authenticationToken);

    QVariantMap windowProperties() const;
    QVariant windowProperty(const QString &propertyName) const;
    void setWindowProperty(const QString &name, const QVariant &value, bool writeUpdateToClient = true);

    void setSurfaceCreationFinished(bool isCreated);

    QPoint lastMousePos() const;

    static const struct wl_surface_interface surface_interface;
protected:
    QScopedPointer<SurfacePrivate> d_ptr;
private:
    Q_DISABLE_COPY(Surface)
    static void surface_destroy(struct wl_client *client, struct wl_resource *_surface);
    static void surface_attach(struct wl_client *client, struct wl_resource *surface,
                        struct wl_resource *buffer, int x, int y);
    static void surface_damage(struct wl_client *client, struct wl_resource *_surface,
                        int32_t x, int32_t y, int32_t width, int32_t height);
    static void surface_frame(struct wl_client *client, struct wl_resource *resource,
                       uint32_t callback);

    static void surface_resource_destory(struct wl_resource *resource);

};

}

#endif //WL_SURFACE_H
