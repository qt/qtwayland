// Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSURFACE_H
#define QWAYLANDSURFACE_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>
#include <QtWaylandCompositor/qwaylandclient.h>
#include <QtWaylandCompositor/qwaylanddrag.h>

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtCore/QVariantMap>

struct wl_client;
struct wl_resource;

QT_BEGIN_NAMESPACE

class QTouchEvent;
class QWaylandSurfacePrivate;
class QWaylandBufferRef;
class QWaylandView;
class QWaylandInputMethodControl;

class QWaylandSurfaceRole
{
public:
    QWaylandSurfaceRole(const QByteArray &n) : m_name(n) {}

    const QByteArray name() { return m_name; }

private:
    QByteArray m_name;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandSurface : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandSurface)
    Q_PROPERTY(QWaylandClient *client READ client CONSTANT)
    Q_PROPERTY(QRectF sourceGeometry READ sourceGeometry NOTIFY sourceGeometryChanged REVISION(1, 13))
    Q_PROPERTY(QSize destinationSize READ destinationSize NOTIFY destinationSizeChanged REVISION(1, 13))
    Q_PROPERTY(QSize bufferSize READ bufferSize NOTIFY bufferSizeChanged REVISION(1, 13))
    Q_PROPERTY(int bufferScale READ bufferScale NOTIFY bufferScaleChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QWaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool hasContent READ hasContent NOTIFY hasContentChanged)
    Q_PROPERTY(bool cursorSurface READ isCursorSurface WRITE markAsCursorSurface NOTIFY cursorSurfaceChanged)
    Q_PROPERTY(bool inhibitsIdle READ inhibitsIdle NOTIFY inhibitsIdleChanged REVISION(1, 14))
    Q_PROPERTY(bool isOpaque READ isOpaque NOTIFY isOpaqueChanged REVISION(6, 4))
    Q_MOC_INCLUDE("qwaylanddrag.h")
    Q_MOC_INCLUDE("qwaylandcompositor.h")

    QML_NAMED_ELEMENT(WaylandSurfaceBase)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead")
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
    bool isOpaque() const;

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
    Q_REVISION(1, 13) void sourceGeometryChanged();
    Q_REVISION(1, 13) void destinationSizeChanged();
    Q_REVISION(1, 13) void bufferSizeChanged();
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
    Q_REVISION(6, 4) void isOpaqueChanged();

    void configure(bool hasBuffer);
    void redraw();
};

QT_END_NAMESPACE

#endif // QWAYLANDSURFACE_H
