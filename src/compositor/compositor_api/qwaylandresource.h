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

#ifndef QWAYLANDRESOURCE_H
#define QWAYLANDRESOURCE_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandResource
{
    Q_GADGET
    QML_NAMED_ELEMENT(waylandresource)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandResource();
    explicit QWaylandResource(wl_resource *resource);

    wl_resource *resource() const { return m_resource; }

private:
    wl_resource *m_resource = nullptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QWaylandResource)

#endif  /*QWAYLANDRESOURCE_H*/
