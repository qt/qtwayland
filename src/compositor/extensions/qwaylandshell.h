/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QWAYLANDSHELL_H
#define QWAYLANDSHELL_H

#include <QtWaylandCompositor/QWaylandExtension>

QT_BEGIN_NAMESPACE

class QWaylandShellPrivate;
class QWaylandShellSurfacePrivate;
class QWaylandSurface;
class QWaylandClient;
class QWaylandInputDevice;
class QWaylandOutput;

class Q_COMPOSITOR_EXPORT QWaylandShell : public QWaylandExtensionTemplate<QWaylandShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandShell)
public:
    QWaylandShell();
    QWaylandShell(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void createShellSurface(QWaylandSurface *surface, QWaylandClient *client, uint id);
};

class Q_COMPOSITOR_EXPORT QWaylandShellSurface : public QWaylandExtensionTemplate<QWaylandShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandShellSurface)
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
        DefaultEdge     = 0x00,
        TopEdge         = 0x01,
        BottomEdge      = 0x02,
        LeftEdge        = 0x04,
        TopLeftEdge     = 0x05,
        BottomLeftEdge  = 0x06,
        RightEdge       = 0x08,
        TopRightEdge    = 0x09,
        BottomRightEdge = 0x10
    };
    Q_ENUM(ResizeEdge);

    enum FocusPolicy{
        DefaultFocus,
        NoKeyboardFocus
    };
    Q_ENUM(FocusPolicy)

    QWaylandShellSurface();
    QWaylandShellSurface(QWaylandShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id);

    Q_INVOKABLE void initialize(QWaylandShell *shell, QWaylandSurface *surface, QWaylandClient *client, uint id);

    QString title() const;
    QString className() const;

    QWaylandSurface *surface() const;

    FocusPolicy focusPolicy() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
Q_SIGNALS:
    void titleChanged();
    void classNameChanged();
    void focusPolicyChanged();
    void pong();
    void startMove(QWaylandInputDevice *inputDevice);
    void startResize(QWaylandInputDevice *inputDevice, ResizeEdge edge);

    void setDefaultToplevel();
    void setTransient(QWaylandSurface *parentSurface, const QPoint &relativeToParent, FocusPolicy focusPolicy);
    void setFullScreen(FullScreenMethod method, uint framerate, QWaylandOutput *output);
    void setPopup(QWaylandInputDevice *inputDevice, QWaylandSurface *parent, const QPoint &relativeToParent);
    void setMaximized(QWaylandOutput *output);

private:
    void initialize();
};

QT_END_NAMESPACE

#endif  /*QWAYLANDSHELL_H*/
