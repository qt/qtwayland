// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDWLSHELL_H
#define QWAYLANDWLSHELL_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandShell>
#include <QtWaylandCompositor/QWaylandShellSurface>
#if QT_CONFIG(wayland_compositor_quick)
#include <QtWaylandCompositor/qwaylandquickchildren.h>
#endif

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandWlShellPrivate;
class QWaylandWlShellSurfacePrivate;
class QWaylandSurface;
class QWaylandClient;
class QWaylandSeat;
class QWaylandOutput;
class QWaylandSurfaceRole;
class QWaylandWlShellSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandWlShell : public QWaylandShellTemplate<QWaylandWlShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWlShell)
public:
    QWaylandWlShell();
    QWaylandWlShell(QWaylandCompositor *compositor);

    void initialize() override;
    QList<QWaylandWlShellSurface *> shellSurfaces() const;
    QList<QWaylandWlShellSurface *> shellSurfacesForClient(QWaylandClient* client) const;
    QList<QWaylandWlShellSurface *> mappedPopups() const;
    QWaylandClient *popupClient() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    void closeAllPopups();

Q_SIGNALS:
    void wlShellSurfaceRequested(QWaylandSurface *surface, const QWaylandResource &resource);
    void wlShellSurfaceCreated(QWaylandWlShellSurface *shellSurface);
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandWlShellSurface : public QWaylandShellSurfaceTemplate<QWaylandWlShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWlShellSurface)
#if QT_CONFIG(wayland_compositor_quick)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandWlShellSurface)
#endif
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandWlShell *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_MOC_INCLUDE("qwaylandsurface.h")

public:
    enum FullScreenMethod {
        DefaultFullScreen,
        ScaleFullScreen,
        DriverFullScreen,
        FillFullScreen
    };
    Q_ENUM(FullScreenMethod);

    enum ResizeEdge {
        NoneEdge        =  0,
        TopEdge         =  1,
        BottomEdge      =  2,
        LeftEdge        =  4,
        TopLeftEdge     =  5,
        BottomLeftEdge  =  6,
        RightEdge       =  8,
        TopRightEdge    =  9,
        BottomRightEdge = 10
    };
    Q_ENUM(ResizeEdge);

    QWaylandWlShellSurface();
    QWaylandWlShellSurface(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &resource);
    ~QWaylandWlShellSurface() override;

    Q_INVOKABLE void initialize(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &resource);

    QString title() const;
    QString className() const;

    QWaylandSurface *surface() const;
    QWaylandWlShell *shell() const;

    Qt::WindowType windowType() const override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();

    static QWaylandWlShellSurface *fromResource(wl_resource *res);

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, ResizeEdge edges);
    Q_INVOKABLE void sendConfigure(const QSize &size, ResizeEdge edges);
    Q_INVOKABLE void sendPopupDone();

#if QT_CONFIG(wayland_compositor_quick)
    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;
#endif

public Q_SLOTS:
    void ping();

Q_SIGNALS:
    void surfaceChanged();
    void shellChanged();
    void titleChanged();
    void classNameChanged();
    void pong();
    void startMove(QWaylandSeat *seat);
    void startResize(QWaylandSeat *seat, ResizeEdge edges);

    void setDefaultToplevel();
    void setTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive);
    void setFullScreen(FullScreenMethod method, uint framerate, QWaylandOutput *output);
    void setPopup(QWaylandSeat *seat, QWaylandSurface *parentSurface, const QPoint &relativeToParent);
    void setMaximized(QWaylandOutput *output);

private:
    void initialize() override;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDWLSHELL_H*/
