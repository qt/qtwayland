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

#ifndef QWAYLANDSURFACE_H
#define QWAYLANDSURFACE_H

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtCore/QVariantMap>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

QT_BEGIN_NAMESPACE

class QTouchEvent;
class QWaylandSurfacePrivate;
class QWaylandCompositor;

#ifdef QT_COMPOSITOR_QUICK
class QWaylandSurfaceItem;
#endif

namespace QtWayland {
class Surface;
class SurfacePrivate;
class ExtendedSurface;
}

class Q_COMPOSITOR_EXPORT QWaylandSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurface)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos NOTIFY posChanged)
    Q_PROPERTY(QWaylandSurface::WindowFlags windowFlags READ windowFlags NOTIFY windowFlagsChanged)
    Q_PROPERTY(QWaylandSurface::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(Qt::ScreenOrientations orientationUpdateMask READ orientationUpdateMask NOTIFY orientationUpdateMaskChanged)
    Q_PROPERTY(QWindow::Visibility visibility READ visibility WRITE setVisibility NOTIFY visibilityChanged)
#ifdef QT_COMPOSITOR_QUICK
    Q_PROPERTY(QObject * windowProperties READ windowPropertyMap CONSTANT)
#endif

    Q_ENUMS(WindowFlag WindowType)
    Q_FLAGS(WindowFlag WindowFlags)

public:
    enum WindowFlag {
        OverridesSystemGestures     = 0x0001,
        StaysOnTop                  = 0x0002,
        BypassWindowManager         = 0x0004
    };
    Q_DECLARE_FLAGS(WindowFlags, WindowFlag)

    enum WindowType {
        None,
        Toplevel,
        Transient,
        Popup
    };

    enum Type {
        Invalid,
        Shm,
        Texture
    };

    QWaylandSurface(QtWayland::Surface *surface = 0);

    WaylandClient *client() const;

    QWaylandSurface *parentSurface() const;
    QLinkedList<QWaylandSurface *> subSurfaces() const;

    Type type() const;
    bool isYInverted() const;

    bool visible() const;

    QPointF pos() const;
    void setPos(const QPointF &pos);
    QSize size() const;
    Q_INVOKABLE void requestSize(const QSize &size);

    Qt::ScreenOrientations orientationUpdateMask() const;
    Qt::ScreenOrientation contentOrientation() const;

    WindowFlags windowFlags() const;

    WindowType windowType() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    GLuint texture() const;
#else
    uint texture() const;
#endif

    QWindow::Visibility visibility() const;
    void setVisibility(QWindow::Visibility visibility);
    Q_INVOKABLE void sendOnScreenVisibilityChange(bool visible); // Compat

    void frameFinished();

    QWaylandSurface *transientParent() const;

    QtWayland::Surface *handle() const;

#ifdef QT_COMPOSITOR_QUICK
    QWaylandSurfaceItem *surfaceItem() const;
    void setSurfaceItem(QWaylandSurfaceItem *surfaceItem);

    QObject *windowPropertyMap() const;
#endif

    qint64 processId() const;
    QByteArray authenticationToken() const;
    QVariantMap windowProperties() const;
    void setWindowProperty(const QString &name, const QVariant &value);

    QPointF mapToParent(const QPointF &) const;
    QPointF mapTo(QWaylandSurface *, const QPointF &) const;

    QWaylandCompositor *compositor() const;

    QString className() const;

    QString title() const;

    bool hasShellSurface() const;
    bool hasInputPanelSurface() const;

    bool transientInactive() const;

    Q_INVOKABLE void destroySurface();
    Q_INVOKABLE void destroySurfaceByForce();
    Q_INVOKABLE void ping();

    void advanceBufferQueue();

public slots:
    void updateSelection();

signals:
    void mapped();
    void unmapped();
    void damaged(const QRect &rect);
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void sizeChanged();
    void posChanged();
    void windowPropertyChanged(const QString &name, const QVariant &value);
    void windowFlagsChanged(WindowFlags flags);
    void windowTypeChanged(WindowType type);
    void contentOrientationChanged();
    void orientationUpdateMaskChanged();
    void extendedSurfaceReady();
    void classNameChanged();
    void titleChanged();
    void raiseRequested();
    void lowerRequested();
    void visibilityChanged();
    void pong();
};

QT_END_NAMESPACE

#endif // QWAYLANDSURFACE_H
