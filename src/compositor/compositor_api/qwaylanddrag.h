/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef QWAYLANDDRAG_H
#define QWAYLANDDRAG_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

#include <QtCore/QObject>
#include <QtCore/QPointF>

QT_REQUIRE_CONFIG(draganddrop);

QT_BEGIN_NAMESPACE

class QWaylandDragPrivate;
class QWaylandSurface;
class QWaylandSeat;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandDrag : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandDrag)

    Q_PROPERTY(QWaylandSurface *icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY iconChanged)
    Q_MOC_INCLUDE("qwaylandsurface.h")

    QML_NAMED_ELEMENT(WaylandDrag)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    explicit QWaylandDrag(QWaylandSeat *seat);

    QWaylandSurface *icon() const;
    QWaylandSurface *origin() const;
    QWaylandSeat *seat() const;
    bool visible() const;

public Q_SLOTS:
    void dragMove(QWaylandSurface *target, const QPointF &pos);
    void drop();
    void cancelDrag();

Q_SIGNALS:
    void iconChanged();
    void dragStarted(); // QWaylandSurface *icon????
};

QT_END_NAMESPACE

#endif // QWAYLANDDRAG_H
