/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QWAYLANDXDGSHELL_H
#define QWAYLANDXDGSHELL_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandShellSurface>

#include <QtCore/QRect>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandXdgShellPrivate;
class QWaylandXdgSurface;
class QWaylandXdgSurfacePrivate;
class QWaylandXdgPopup;
class QWaylandXdgPopupPrivate;

class QWaylandSurface;
class QWaylandSurfaceRole;
class QWaylandInputDevice;
class QWaylandOutput;
class QWaylandClient;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgShell : public QWaylandCompositorExtensionTemplate<QWaylandXdgShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgShell)
public:
    QWaylandXdgShell();
    QWaylandXdgShell(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    uint ping(QWaylandClient *client);
    void closeAllPopups();

Q_SIGNALS:
    void createXdgSurface(QWaylandSurface *surface, const QWaylandResource &resource);
    void xdgSurfaceCreated(QWaylandXdgSurface *xdgSurface);
    void xdgPopupCreated(QWaylandXdgPopup *xdgPopup);
    void createXdgPopup(QWaylandSurface *surface, QWaylandSurface *parent, QWaylandInputDevice *seat, const QPoint &position, const QWaylandResource &resource);
    void pong(uint serial);

private Q_SLOTS:
    void handleDefaultInputDeviceChanged(QWaylandInputDevice *newDevice, QWaylandInputDevice *oldDevice);
    void handleFocusChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);

};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgSurface : public QWaylandShellSurfaceTemplate<QWaylandXdgSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgSurface)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandXdgSurface *parentSurface READ parentSurface NOTIFY parentSurfaceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)

    Q_PROPERTY(QList<int> states READ statesAsInts NOTIFY statesChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool resizing READ resizing NOTIFY resizingChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)

public:
    enum State : uint {
        MaximizedState  = 1,
        FullscreenState = 2,
        ResizingState   = 3,
        ActivatedState  = 4
    };
    Q_ENUM(State)

    enum ResizeEdge : uint {
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
    Q_ENUM(ResizeEdge)

    QWaylandXdgSurface();
    QWaylandXdgSurface(QWaylandXdgShell* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandXdgShell* xdgShell, QWaylandSurface *surface, const QWaylandResource &resource);

    QString title() const;
    QString appId() const;
    QRect windowGeometry() const;
    QVector<uint> states() const;
    bool maximized() const;
    bool fullscreen() const;
    bool resizing() const;
    bool activated() const;

    QWaylandSurface *surface() const;
    QWaylandXdgSurface *parentSurface() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();
    static QWaylandXdgSurface *fromResource(::wl_resource *resource);

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, ResizeEdge edge);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QVector<uint> &states);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QVector<State> &states);
    Q_INVOKABLE void sendClose();

    Q_INVOKABLE uint requestMaximized(const QSize &size);
    Q_INVOKABLE uint requestUnMaximized(const QSize &size = QSize(0, 0));
    Q_INVOKABLE uint requestFullscreen(const QSize &size);
    Q_INVOKABLE uint requestResizing(const QSize &maxSize);

    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void surfaceChanged();
    void titleChanged();
    void windowGeometryChanged();
    void appIdChanged();
    void parentSurfaceChanged();

    void statesChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void resizingChanged();
    void activatedChanged();

    void showWindowMenu(QWaylandInputDevice *inputDevice, const QPoint &localSurfacePosition);
    void startMove(QWaylandInputDevice *inputDevice);
    void startResize(QWaylandInputDevice *inputDevice, ResizeEdge edges);
    void setMaximized();
    void unsetMaximized();
    void setFullscreen(QWaylandOutput *output);
    void unsetFullscreen();
    void setMinimized();
    void ackConfigure(uint serial);

private:
    void initialize();
    QList<int> statesAsInts() const;

private Q_SLOTS:
    void handleSurfaceSizeChanged();
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgPopup : public QWaylandCompositorExtensionTemplate<QWaylandXdgPopup>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgPopup)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QWaylandSurface *parentSurface READ parentSurface NOTIFY parentSurfaceChanged)

public:
    QWaylandXdgPopup();
    QWaylandXdgPopup(QWaylandXdgShell *xdgShell, QWaylandSurface *surface, QWaylandSurface *parentSurface, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandXdgShell *shell, QWaylandSurface *surface,
                                QWaylandSurface *parentSurface, const QWaylandResource &resource);

    QWaylandSurface *surface() const;
    QWaylandSurface *parentSurface() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();
    static QWaylandXdgPopup *fromResource(::wl_resource *resource);

    Q_INVOKABLE void sendPopupDone();

Q_SIGNALS:
    void surfaceChanged();
    void parentSurfaceChanged();

private:
    void initialize();
};

QT_END_NAMESPACE

#endif  /*QWAYLANDXDGSHELL_H*/
