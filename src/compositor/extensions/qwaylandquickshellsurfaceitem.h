// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKSHELLSURFACEITEM_H
#define QWAYLANDQUICKSHELLSURFACEITEM_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandQuickItem>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QWaylandQuickShellSurfaceItemPrivate;
class QWaylandShellSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickShellSurfaceItem : public QWaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQuickShellSurfaceItem)
    Q_PROPERTY(QWaylandShellSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)
    Q_PROPERTY(bool autoCreatePopupItems READ autoCreatePopupItems WRITE setAutoCreatePopupItems NOTIFY autoCreatePopupItemsChanged)
    Q_PROPERTY(bool staysOnTop READ staysOnTop WRITE setStaysOnTop NOTIFY staysOnTopChanged)
    Q_PROPERTY(bool staysOnBottom READ staysOnBottom WRITE setStaysOnBottom NOTIFY staysOnBottomChanged)
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

    bool staysOnTop() const;
    void setStaysOnTop(bool on);
    bool staysOnBottom() const;
    void setStaysOnBottom(bool on);

Q_SIGNALS:
    void shellSurfaceChanged();
    void moveItemChanged();
    void autoCreatePopupItemsChanged();
    void staysOnTopChanged();
    void staysOnBottomChanged();

protected:
    QWaylandQuickShellSurfaceItem(QWaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent);
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSHELLSURFACEITEM_H
