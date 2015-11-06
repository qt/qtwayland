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

#ifndef QWAYLANDQUICKSHELLSURFACEITEM_H
#define QWAYLANDQUICKSHELLSURFACEITEM_H

#include <QtWaylandCompositor/QWaylandExtension>
#include <QtWaylandCompositor/QWaylandQuickItem>
#include <QtWaylandCompositor/QWaylandShellSurface>

QT_BEGIN_NAMESPACE

class QWaylandQuickShellSurfaceItemPrivate;

class Q_COMPOSITOR_EXPORT QWaylandQuickShellSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickShellSurfaceItem)
    Q_PROPERTY(QWaylandShellSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)

public:
    QWaylandQuickShellSurfaceItem(QQuickItem *parent = 0);

    static QWaylandQuickShellSurfaceItemPrivate *get(QWaylandQuickShellSurfaceItem *item) { return item->d_func(); }

    QWaylandShellSurface *shellSurface() const;
    void setShellSurface(QWaylandShellSurface *shellSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);
Q_SIGNALS:
    void shellSurfaceChanged();
    void moveItemChanged();

private Q_SLOTS:
    void handleStartMove(QWaylandInputDevice *inputDevice);
    void handleStartResize(QWaylandInputDevice *inputDevice, QWaylandShellSurface::ResizeEdge edges);
    void adjustOffsetForNextFrame(const QPointF &offset);
protected:
    QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent);

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface) Q_DECL_OVERRIDE;

    void componentComplete() Q_DECL_OVERRIDE;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDQUICKSHELLSURFACEITEM_H*/
