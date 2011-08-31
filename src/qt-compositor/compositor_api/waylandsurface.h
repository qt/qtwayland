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

#ifndef WAYLANDSURFACE_H
#define WAYLANDSURFACE_H

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtCore/QVariantMap>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtOpenGL/QGLContext>
#endif

class WaylandSurfacePrivate;

namespace Wayland {
class Surface;
class SurfacePrivate;
}

class WaylandSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurface)
public:
    enum Type {
        Invalid,
        Shm,
        Texture,
        Direct
    };

    WaylandSurface(Wayland::Surface *surface);

    Type type() const;
    bool isYInverted() const;

    void setGeometry(const QRect &geometry);
    QRect geometry() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture() const;
#else
    uint texture() const;
#endif

    void sendMousePressEvent(const QPoint &pos, Qt::MouseButton button);
    void sendMouseReleaseEvent(const QPoint &pos, Qt::MouseButton button);
    void sendMouseMoveEvent(const QPoint &pos);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendOnScreenVisibilityChange(bool visible);

    void frameFinished();
    void setInputFocus();

    Wayland::Surface *handle() const;
    qint64 processId() const;
    QByteArray authenticationToken() const;
    QVariantMap windowProperties() const;
    void setWindowProperty(const QString &name, const QVariant &value);

signals:
    void mapped(const QSize &size);
    void damaged(const QRect &rect);

    friend class Wayland::Surface;
    friend class Wayland::SurfacePrivate;
};

#endif // WAYLANDSURFACE_H
