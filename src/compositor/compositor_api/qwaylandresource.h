// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDRESOURCE_H
#define QWAYLANDRESOURCE_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

struct wl_resource;

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandResource
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
