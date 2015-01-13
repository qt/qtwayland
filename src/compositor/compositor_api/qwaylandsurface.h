/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QWAYLANDSURFACE_H
#define QWAYLANDSURFACE_H

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtCore/QVariantMap>

struct wl_client;
struct wl_resource;

QT_BEGIN_NAMESPACE

class QTouchEvent;
class QWaylandClient;
class QWaylandSurfacePrivate;
class QWaylandCompositor;
class QWaylandBufferRef;
class QWaylandSurfaceView;
class QWaylandSurfaceInterface;
class QWaylandSurfaceOp;
class QWaylandOutput;

namespace QtWayland {
class Surface;
class SurfacePrivate;
class ExtendedSurface;
}

class Q_COMPOSITOR_EXPORT QWaylandBufferAttacher
{
public:
    virtual ~QWaylandBufferAttacher() {}

protected:
    virtual void attach(const QWaylandBufferRef &ref) = 0;
    virtual void unmap() = 0;

    friend class QtWayland::Surface;
};

class QWaylandSurfaceEnterEventPrivate;

class Q_COMPOSITOR_EXPORT QWaylandSurfaceEnterEvent : public QEvent
{
public:
    QWaylandSurfaceEnterEvent(QWaylandOutput *output);
    ~QWaylandSurfaceEnterEvent();

    QWaylandOutput *output() const;

    static const QEvent::Type WaylandSurfaceEnter;

private:
    QWaylandSurfaceEnterEventPrivate *d;
};

class QWaylandSurfaceLeaveEventPrivate;

class Q_COMPOSITOR_EXPORT QWaylandSurfaceLeaveEvent : public QEvent
{
public:
    QWaylandSurfaceLeaveEvent(QWaylandOutput *output);
    ~QWaylandSurfaceLeaveEvent();

    QWaylandOutput *output() const;

    static const QEvent::Type WaylandSurfaceLeave;

private:
    QWaylandSurfaceLeaveEventPrivate *d;
};

class Q_COMPOSITOR_EXPORT QWaylandSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurface)
    Q_PROPERTY(QWaylandClient *client READ client CONSTANT)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QWaylandSurface::WindowFlags windowFlags READ windowFlags NOTIFY windowFlagsChanged)
    Q_PROPERTY(QWaylandSurface::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(Qt::ScreenOrientations orientationUpdateMask READ orientationUpdateMask NOTIFY orientationUpdateMaskChanged)
    Q_PROPERTY(QWindow::Visibility visibility READ visibility WRITE setVisibility NOTIFY visibilityChanged)
    Q_PROPERTY(QWaylandSurface *transientParent READ transientParent)
    Q_PROPERTY(QPointF transientOffset READ transientOffset)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(bool isYInverted READ isYInverted NOTIFY yInvertedChanged)

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

    QWaylandSurface(wl_client *client, quint32 id, int version, QWaylandCompositor *compositor);
    virtual ~QWaylandSurface();

    QWaylandClient *client() const;

    QWaylandSurface *parentSurface() const;
    QLinkedList<QWaylandSurface *> subSurfaces() const;
    void addInterface(QWaylandSurfaceInterface *interface);
    void removeInterface(QWaylandSurfaceInterface *interface);

    Type type() const;
    bool isYInverted() const;

    bool visible() const;
    bool isMapped() const;

    QSize size() const;
    Q_INVOKABLE void requestSize(const QSize &size);

    Qt::ScreenOrientations orientationUpdateMask() const;
    Qt::ScreenOrientation contentOrientation() const;

    WindowFlags windowFlags() const;

    WindowType windowType() const;

    QWindow::Visibility visibility() const;
    void setVisibility(QWindow::Visibility visibility);
    Q_INVOKABLE void sendOnScreenVisibilityChange(bool visible); // Compat

    QWaylandSurface *transientParent() const;

    QPointF transientOffset() const;

    QtWayland::Surface *handle();

    QByteArray authenticationToken() const;
    QVariantMap windowProperties() const;
    void setWindowProperty(const QString &name, const QVariant &value);

    QWaylandCompositor *compositor() const;

    QWaylandOutput *mainOutput() const;
    void setMainOutput(QWaylandOutput *mainOutput);

    QList<QWaylandOutput *> outputs() const;

    QString className() const;

    QString title() const;

    bool hasInputPanelSurface() const;

    bool transientInactive() const;

    bool inputRegionContains(const QPoint &p) const;

    Q_INVOKABLE void destroy();
    Q_INVOKABLE void destroySurface();
    Q_INVOKABLE void ping();

    void ref();
    void setMapped(bool mapped);

    void setBufferAttacher(QWaylandBufferAttacher *attacher);
    QWaylandBufferAttacher *bufferAttacher() const;

    QList<QWaylandSurfaceView *> views() const;
    QList<QWaylandSurfaceInterface *> interfaces() const;

    QWaylandSurfaceView *shellView() const;

    bool sendInterfaceOp(QWaylandSurfaceOp &op);

    static QWaylandSurface *fromResource(::wl_resource *resource);

public Q_SLOTS:
    void updateSelection();

protected:
    QWaylandSurface(QWaylandSurfacePrivate *dptr);

Q_SIGNALS:
    void mapped();
    void unmapped();
    void damaged(const QRegion &rect);
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void sizeChanged();
    void offsetForNextFrame(const QPoint &offset);
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
    void surfaceDestroyed();
    void shellViewCreated();
    void outputChanged(QWaylandOutput *newOutput, QWaylandOutput *oldOutput);
    void yInvertedChanged();

    void configure(bool hasBuffer);
    void redraw();

    friend class QWaylandSurfaceView;
    friend class QWaylandSurfaceInterface;
    friend class QtWayland::Surface;
};

class QWaylandUnmapLockPrivate;
class Q_COMPOSITOR_EXPORT QWaylandUnmapLock
{
public:
    QWaylandUnmapLock(QWaylandSurface *surface);
    ~QWaylandUnmapLock();

private:
    QWaylandUnmapLockPrivate *const d;
};

QT_END_NAMESPACE

#endif // QWAYLANDSURFACE_H
