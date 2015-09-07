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

class QWaylandShellSurface;
class QWaylandShellSurfacePrivate;
class QWaylandSurface;
class QWaylandView;
class QWaylandShellPrivate;
class QWaylandClient;

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
    Q_PROPERTY(SurfaceType surfaceType READ surfaceType NOTIFY surfaceTypeChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QWaylandView *view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(QWaylandSurface *transientParent READ transientParent NOTIFY transientParentChanged)
    Q_PROPERTY(QWaylandSurface *surface READ surface CONSTANT)

public:
    enum SurfaceType {
        None,
        Toplevel,
        Transient,
        Popup
    };

    QWaylandShellSurface();
    QWaylandShellSurface(QWaylandShell *shell, QWaylandSurface *surface, QWaylandView *view, QWaylandClient *client, uint id);

    Q_INVOKABLE void initialize(QWaylandShell *shell, QWaylandSurface *surface, QWaylandView *view, QWaylandClient *client, uint id);

    SurfaceType surfaceType() const;

    QWaylandView *view() const;
    void setView(QWaylandView *view);

    QString title() const;
    QString className() const;

    QWaylandSurface *surface() const;

    QWaylandSurface *transientParent() const;
    QPointF transientOffset() const;

    bool isTransientInactive() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
Q_SIGNALS:
    void surfaceTypeChanged();
    void viewChanged();
    void titleChanged();
    void classNameChanged();
    void transientParentChanged();
    void pong();

private Q_SLOTS:
    void mappedChanged();
    void adjustOffset(const QPoint &p);
private:
    void initialize();
};

QT_END_NAMESPACE

#endif  /*QWAYLANDSHELL_H*/
