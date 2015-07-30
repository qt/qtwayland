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
#include <QtCompositor/qwaylandextension.h>

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
class QWaylandView;
class QWaylandSurfaceOp;
class QWaylandOutput;

namespace QtWayland {
class Surface;
class SurfacePrivate;
class ExtendedSurface;
}

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

class Q_COMPOSITOR_EXPORT QWaylandSurface : public QObject, public QWaylandExtensionContainer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurface)
    Q_PROPERTY(QWaylandClient *client READ client CONSTANT)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QWaylandOutput *primaryOutput READ primaryOutput WRITE setPrimaryOutput NOTIFY primaryOutputChanged)
    Q_PROPERTY(QWaylandSurface::Origin origin READ origin NOTIFY originChanged);

public:
    enum Origin {
        OriginTopLeft,
        OriginBottomLeft
    };

    QWaylandSurface(wl_client *client, quint32 id, int version, QWaylandCompositor *compositor);
    virtual ~QWaylandSurface();

    QWaylandClient *client() const;

    QWaylandSurface *parentSurface() const;
    QLinkedList<QWaylandSurface *> subSurfaces() const;

    bool visible() const;
    bool isMapped() const;

    QSize size() const;

    Qt::ScreenOrientation contentOrientation() const;

    Origin origin() const;

    QtWayland::Surface *handle();

    QByteArray authenticationToken() const;

    QWaylandCompositor *compositor() const;

    QWaylandOutput *primaryOutput() const;
    void setPrimaryOutput(QWaylandOutput *output);

    QString className() const;

    QString title() const;

    bool hasInputPanelSurface() const;

    bool inputRegionContains(const QPoint &p) const;

    Q_INVOKABLE void destroy();

    Q_INVOKABLE void sendFrameCallbacks();

    void ref();
    void deref();
    void setMapped(bool mapped);

    QList<QWaylandView *> views() const;

    static QWaylandSurface *fromResource(::wl_resource *resource);
    struct wl_resource *resource() const;

    void enter(QWaylandOutput *output);
    void leave(QWaylandOutput *output);

    void markAsCursorSurface(bool cursorSurface);
    bool isCursorSurface() const;

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
    void contentOrientationChanged();
    void extendedSurfaceReady();
    void classNameChanged();
    void titleChanged();
    void raiseRequested();
    void lowerRequested();
    void pong();
    void surfaceDestroyed();
    void primaryOutputChanged(QWaylandOutput *newOutput, QWaylandOutput *oldOutput);
    void originChanged();

    void configure(bool hasBuffer);
    void redraw();

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
