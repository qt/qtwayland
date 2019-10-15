/****************************************************************************
**
** Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSURFACE_H
#define QWAYLANDSURFACE_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>
#include <QtWaylandCompositor/qwaylandclient.h>

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
class QWaylandInputMethodControl;
class QWaylandDrag;

class QWaylandSurfaceRole
{
public:
    QWaylandSurfaceRole(const QByteArray &n) : m_name(n) {}

    const QByteArray name() { return m_name; }

private:
    QByteArray m_name;
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandSurface : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurface)
    Q_PROPERTY(QWaylandClient *client READ client CONSTANT)
    Q_PROPERTY(QRectF sourceGeometry READ sourceGeometry NOTIFY sourceGeometryChanged REVISION 13)
    Q_PROPERTY(QSize destinationSize READ destinationSize NOTIFY destinationSizeChanged REVISION 13)
    Q_PROPERTY(QSize bufferSize READ bufferSize NOTIFY bufferSizeChanged REVISION 13)
#if QT_DEPRECATED_SINCE(5, 13)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged) // Qt 6: Remove
#endif
    Q_PROPERTY(int bufferScale READ bufferScale NOTIFY bufferScaleChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QWaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool hasContent READ hasContent NOTIFY hasContentChanged)
    Q_PROPERTY(bool cursorSurface READ isCursorSurface WRITE markAsCursorSurface NOTIFY cursorSurfaceChanged)
    Q_PROPERTY(bool inhibitsIdle READ inhibitsIdle NOTIFY inhibitsIdleChanged REVISION 14)

public:
    enum Origin {
        OriginTopLeft,
        OriginBottomLeft
    };
    Q_ENUM(Origin)

    QWaylandSurface();
    QWaylandSurface(QWaylandCompositor *compositor, QWaylandClient *client, uint id, int version);
    ~QWaylandSurface() override;

    Q_INVOKABLE void initialize(QWaylandCompositor *compositor, QWaylandClient *client, uint id, int version);
    bool isInitialized() const;

    QWaylandClient *client() const;
    ::wl_client *waylandClient() const;

    bool setRole(QWaylandSurfaceRole *role, wl_resource *errorResource, uint32_t errorCode);
    QWaylandSurfaceRole *role() const;

    bool hasContent() const;

    QRectF sourceGeometry() const;
    QSize destinationSize() const;
#if QT_DEPRECATED_SINCE(5, 13)
    QT_DEPRECATED QSize size() const;
#endif
    QSize bufferSize() const;
    int bufferScale() const;

    Qt::ScreenOrientation contentOrientation() const;

    Origin origin() const;

    QWaylandCompositor *compositor() const;

    bool inputRegionContains(const QPoint &p) const;
    bool inputRegionContains(const QPointF &position) const;

    Q_INVOKABLE void destroy();
    Q_INVOKABLE bool isDestroyed() const;

    Q_INVOKABLE void frameStarted();
    Q_INVOKABLE void sendFrameCallbacks();

    QWaylandView *primaryView() const;
    void setPrimaryView(QWaylandView *view);

    QList<QWaylandView *> views() const;

    static QWaylandSurface *fromResource(::wl_resource *resource);
    struct wl_resource *resource() const;

    void markAsCursorSurface(bool cursorSurface);
    bool isCursorSurface() const;

    bool inhibitsIdle() const;

#if QT_CONFIG(im)
    QWaylandInputMethodControl *inputMethodControl() const;
#endif

public Q_SLOTS:
#if QT_CONFIG(clipboard)
    void updateSelection();
#endif

protected:
    QWaylandSurface(QWaylandSurfacePrivate &dptr);

Q_SIGNALS:
    void hasContentChanged();
    void damaged(const QRegion &rect);
    void parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent);
    void childAdded(QWaylandSurface *child);
    Q_REVISION(13) void sourceGeometryChanged();
    Q_REVISION(13) void destinationSizeChanged();
#if QT_DEPRECATED_SINCE(5, 13)
    QT_DEPRECATED void sizeChanged();
#endif
    Q_REVISION(13) void bufferSizeChanged();
    void bufferScaleChanged();
    void offsetForNextFrame(const QPoint &offset);
    void contentOrientationChanged();
    void surfaceDestroyed();
    void originChanged();
    void subsurfacePositionChanged(const QPoint &position);
    void subsurfacePlaceAbove(QWaylandSurface *sibling);
    void subsurfacePlaceBelow(QWaylandSurface *sibling);
    void dragStarted(QWaylandDrag *drag);
    void cursorSurfaceChanged();
    Q_REVISION(14) void inhibitsIdleChanged();

    void configure(bool hasBuffer);
    void redraw();
};

QT_END_NAMESPACE

#endif // QWAYLANDSURFACE_H
