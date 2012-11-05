/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WAYLANDSURFACE_H
#define WAYLANDSURFACE_H

#include "waylandexport.h"

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtCore/QVariantMap>

#include <QtGui/QOpenGLContext>
#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

class QTouchEvent;
class WaylandSurfacePrivate;
class WaylandCompositor;

#ifdef QT_COMPOSITOR_QUICK
class WaylandSurfaceItem;
#endif

namespace Wayland {
class Surface;
class SurfacePrivate;
class ExtendedSurface;
}

class Q_COMPOSITOR_EXPORT WaylandSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurface)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos NOTIFY posChanged)
    Q_PROPERTY(WaylandSurface::WindowFlags windowFlags READ windowFlags NOTIFY windowFlagsChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(Qt::ScreenOrientations orientationUpdateMask READ orientationUpdateMask NOTIFY orientationUpdateMaskChanged)

    Q_ENUMS(WindowFlag)
    Q_FLAGS(WindowFlag WindowFlags)

public:
    enum WindowFlag {
        OverridesSystemGestures     = 0x0001,
        StaysOnTop                  = 0x0002
    };
    Q_DECLARE_FLAGS(WindowFlags, WindowFlag)

    enum Type {
        Invalid,
        Shm,
        Texture
    };

    WaylandSurface(Wayland::Surface *surface = 0);

    WaylandClient *client() const;

    WaylandSurface *parentSurface() const;
    QLinkedList<WaylandSurface *> subSurfaces() const;

    Type type() const;
    bool isYInverted() const;

    bool visible() const;

    QPointF pos() const;
    void setPos(const QPointF &pos);
    QSize size() const;
    void setSize(const QSize &size);
    void sendConfigure(const QSize &size);

    Qt::ScreenOrientations orientationUpdateMask() const;
    Qt::ScreenOrientation contentOrientation() const;

    WindowFlags windowFlags() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture(QOpenGLContext *context) const;
#else
    uint texture(QOpenGLContext *context) const;
#endif

    Q_INVOKABLE void sendOnScreenVisibilityChange(bool visible);

    void frameFinished();

    WaylandSurface *transientParent() const;

    Wayland::Surface *handle() const;

#ifdef QT_COMPOSITOR_QUICK
    WaylandSurfaceItem *surfaceItem() const;
    void setSurfaceItem(WaylandSurfaceItem *surfaceItem);
#endif

    qint64 processId() const;
    QByteArray authenticationToken() const;
    QVariantMap windowProperties() const;
    void setWindowProperty(const QString &name, const QVariant &value);

    QPointF mapToParent(const QPointF &) const;
    QPointF mapTo(WaylandSurface *, const QPointF &) const;

    WaylandCompositor *compositor() const;

    QString className() const;

    QString title() const;

    bool transientInactive() const;

signals:
    void mapped();
    void unmapped();
    void damaged(const QRect &rect);
    void parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent);
    void sizeChanged();
    void posChanged();
    void windowPropertyChanged(const QString &name, const QVariant &value);
    void windowFlagsChanged(WindowFlags flags);
    void contentOrientationChanged();
    void orientationUpdateMaskChanged();
    void extendedSurfaceReady();
    void classNameChanged();
    void titleChanged();

};

#endif // WAYLANDSURFACE_H
