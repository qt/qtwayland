/****************************************************************************
**
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

#ifndef QWAYLANDWLSHELL_H
#define QWAYLANDWLSHELL_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandShellSurface>

#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QWaylandWlShellPrivate;
class QWaylandWlShellSurfacePrivate;
class QWaylandSurface;
class QWaylandClient;
class QWaylandInputDevice;
class QWaylandOutput;
class QWaylandSurfaceRole;
class QWaylandWlShellSurface;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandWlShell : public QWaylandCompositorExtensionTemplate<QWaylandWlShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWlShell)
public:
    QWaylandWlShell();
    QWaylandWlShell(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void createShellSurface(QWaylandSurface *surface, const QWaylandResource &resource);
    void shellSurfaceCreated(QWaylandWlShellSurface *shellSurface);
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandWlShellSurface : public QWaylandShellSurfaceTemplate<QWaylandWlShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWlShellSurface)
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(FocusPolicy focusPolicy READ focusPolicy NOTIFY focusPolicyChanged)

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

    enum FocusPolicy{
        DefaultFocus,
        NoKeyboardFocus
    };
    Q_ENUM(FocusPolicy)

    QWaylandWlShellSurface();
    QWaylandWlShellSurface(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &resource);

    Q_INVOKABLE void initialize(QWaylandWlShell *shell, QWaylandSurface *surface, const QWaylandResource &resource);

    QString title() const;
    QString className() const;

    QWaylandSurface *surface() const;

    FocusPolicy focusPolicy() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static QWaylandSurfaceRole *role();

    static QWaylandWlShellSurface *fromResource(wl_resource *res);

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, ResizeEdge edges);
    Q_INVOKABLE void sendConfigure(const QSize &size, ResizeEdge edges);
    Q_INVOKABLE void sendPopupDone();

    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void ping();

Q_SIGNALS:
    void surfaceChanged();
    void titleChanged();
    void classNameChanged();
    void focusPolicyChanged();
    void pong();
    void startMove(QWaylandInputDevice *inputDevice);
    void startResize(QWaylandInputDevice *inputDevice, ResizeEdge edges);

    void setDefaultToplevel();
    void setTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, FocusPolicy focusPolicy);
    void setFullScreen(FullScreenMethod method, uint framerate, QWaylandOutput *output);
    void setPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parentSurface, const QPoint &relativeToParent);
    void setMaximized(QWaylandOutput *output);

private:
    void initialize();
};

QT_END_NAMESPACE

#endif  /*QWAYLANDWLSHELL_H*/
