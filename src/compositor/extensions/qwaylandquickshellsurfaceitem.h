/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QWAYLANDQUICKSHELLSURFACEITEM_H
#define QWAYLANDQUICKSHELLSURFACEITEM_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandQuickItem>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QWaylandQuickShellSurfaceItemPrivate;
class QWaylandShellSurface;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickShellSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickShellSurfaceItem)
    Q_PROPERTY(QWaylandShellSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)
    Q_PROPERTY(bool autoCreatePopupItems READ autoCreatePopupItems WRITE setAutoCreatePopupItems NOTIFY autoCreatePopupItemsChanged)
    Q_MOC_INCLUDE("qwaylandshellsurface.h")
    QML_NAMED_ELEMENT(ShellSurfaceItem)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandQuickShellSurfaceItem(QQuickItem *parent = nullptr);
    ~QWaylandQuickShellSurfaceItem() override;

    QWaylandShellSurface *shellSurface() const;
    void setShellSurface(QWaylandShellSurface *shellSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);

    bool autoCreatePopupItems();
    void setAutoCreatePopupItems(bool enabled);

Q_SIGNALS:
    void shellSurfaceChanged();
    void moveItemChanged();
    void autoCreatePopupItemsChanged();

protected:
    QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent);
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSHELLSURFACEITEM_H
