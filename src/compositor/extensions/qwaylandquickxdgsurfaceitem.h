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

#ifndef QWAYLANDQUICKXDGSURFACEITEM_H
#define QWAYLANDQUICKXDGSURFACEITEM_H

#include <QtWaylandCompositor/QWaylandQuickItem>
#include <QtWaylandCompositor/QWaylandXdgSurface>

QT_BEGIN_NAMESPACE

class QWaylandQuickXdgSurfaceItemPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickXdgSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickXdgSurfaceItem)
    Q_PROPERTY(QWaylandXdgSurface *xdgSurface READ xdgSurface WRITE setXdgSurface NOTIFY xdgSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)

public:
    QWaylandQuickXdgSurfaceItem(QQuickItem *parent = nullptr);

    QWaylandXdgSurface *xdgSurface() const;
    void setXdgSurface(QWaylandXdgSurface *xdgSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);

Q_SIGNALS:
    void xdgSurfaceChanged();
    void moveItemChanged();

private Q_SLOTS:
    void handleStartMove(QWaylandInputDevice *inputDevice);
    void handleStartResize(QWaylandInputDevice *inputDevice, QWaylandXdgSurface::ResizeEdge edges);
    void handleSetMaximized();
    void handleUnsetMaximized();
    void handleMaximizedChanged();
    void handleActivatedChanged();
    void handleSurfaceSizeChanged();

protected:
    QWaylandQuickXdgSurfaceItem(QWaylandQuickXdgSurfaceItemPrivate &dd, QQuickItem *parent);

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface) Q_DECL_OVERRIDE;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDQUICKXDGSURFACEITEM_H*/
