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

#include "qwaylanddrag.h"

#include <private/qobject_p.h>

#include "qwaylandview.h"
#include <QtWaylandCompositor/private/qwaylandseat_p.h>
#include <QtWaylandCompositor/private/qtwaylandcompositorglobal_p.h>

#if QT_CONFIG(wayland_datadevice)
#include "qwldatadevice_p.h"
#endif

QT_BEGIN_NAMESPACE

class QWaylandDragPrivate : public QObjectPrivate
{
public:
    QWaylandDragPrivate(QWaylandSeat *seat)
        : seat(seat)
    {
    }

    QtWayland::DataDevice *dataDevice()
    {
        return QWaylandSeatPrivate::get(seat)->dataDevice();
    }

    const QtWayland::DataDevice *dataDevice() const
    {
        return QWaylandSeatPrivate::get(seat)->dataDevice();
    }

    QWaylandSeat *seat = nullptr;
};

QWaylandDrag::QWaylandDrag(QWaylandSeat *seat)
    : QObject(* new QWaylandDragPrivate(seat))
{
}

QWaylandSurface *QWaylandDrag::icon() const
{
    Q_D(const QWaylandDrag);

    const QtWayland::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return nullptr;

    return dataDevice->dragIcon();
}

QWaylandSurface *QWaylandDrag::origin() const
{
    Q_D(const QWaylandDrag);
    const QtWayland::DataDevice *dataDevice = d->dataDevice();
    return dataDevice ? dataDevice->dragOrigin() : nullptr;
}

QWaylandSeat *QWaylandDrag::seat() const
{
    Q_D(const QWaylandDrag);
    return d->seat;
}


bool QWaylandDrag::visible() const
{
    Q_D(const QWaylandDrag);

    const QtWayland::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return false;

    return dataDevice->dragIcon() != nullptr;
}

void QWaylandDrag::dragMove(QWaylandSurface *target, const QPointF &pos)
{
    Q_D(QWaylandDrag);
    QtWayland::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->dragMove(target, pos);
}
void QWaylandDrag::drop()
{
    Q_D(QWaylandDrag);
    QtWayland::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->drop();
}

void QWaylandDrag::cancelDrag()
{
    Q_D(QWaylandDrag);
    QtWayland::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->cancelDrag();
}

QT_END_NAMESPACE

#include "moc_qwaylanddrag.cpp"
