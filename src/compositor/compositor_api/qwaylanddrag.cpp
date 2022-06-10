// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
