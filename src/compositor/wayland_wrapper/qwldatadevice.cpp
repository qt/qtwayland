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

#include "qwldatadevice_p.h"

#include "qwlcompositor_p.h"
#include "qwldatasource_p.h"
#include "qwldataoffer_p.h"
#include "qwlinputdevice_p.h"
#include "qwlkeyboard_p.h"
#include "qwlpointer_p.h"
#include "qwlsurface_p.h"
#include "qwltouch_p.h"
#include "qwldatadevicemanager_p.h"

#include "qwaylanddrag.h"
#include "qwaylandsurfaceview.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWayland {

DataDevice::DataDevice(InputDevice *inputDevice)
    : wl_data_device()
    , m_compositor(inputDevice->compositor())
    , m_inputDevice(inputDevice)
    , m_selectionSource(0)
    , m_dragClient(0)
    , m_dragDataSource(0)
    , m_dragFocus(0)
    , m_dragFocusResource(0)
    , m_dragIcon(0)
{
}

void DataDevice::setFocus(QtWaylandServer::wl_keyboard::Resource *focusResource)
{
    if (!focusResource)
        return;

    Resource *resource = resourceMap().value(focusResource->client());

    if (!resource)
        return;

    if (m_selectionSource) {
        DataOffer *offer = new DataOffer(m_selectionSource, resource);
        send_selection(resource->handle, offer->resource()->handle);
    }
}

void DataDevice::setDragFocus(QWaylandSurfaceView *focus, const QPointF &localPosition)
{
    if (m_dragFocusResource) {
        send_leave(m_dragFocusResource->handle);
        m_dragFocus = 0;
        m_dragFocusResource = 0;
    }

    if (!focus)
        return;

    if (!m_dragDataSource && m_dragClient != focus->surface()->handle()->resource()->client())
        return;

    Resource *resource = resourceMap().value(focus->surface()->handle()->resource()->client());

    if (!resource)
        return;

    uint32_t serial = wl_display_next_serial(m_compositor->wl_display());

    DataOffer *offer = m_dragDataSource ? new DataOffer(m_dragDataSource, resource) : 0;

    if (m_dragDataSource && !offer)
        return;

    send_enter(resource->handle, serial, focus->surface()->handle()->resource()->handle,
               wl_fixed_from_double(localPosition.x()), wl_fixed_from_double(localPosition.y()),
               offer->resource()->handle);

    m_dragFocus = focus;
    m_dragFocusResource = resource;
}

QWaylandSurfaceView *DataDevice::dragIcon() const
{
    return m_dragIcon;
}

void DataDevice::sourceDestroyed(DataSource *source)
{
    if (m_selectionSource == source)
        m_selectionSource = 0;
}

void DataDevice::focus()
{
    QWaylandSurfaceView *focus = m_compositor->waylandCompositor()->pickView(m_pointer->currentPosition());

    if (focus != m_dragFocus)
        setDragFocus(focus, m_compositor->waylandCompositor()->mapToView(focus, m_pointer->currentPosition()));
}

void DataDevice::motion(uint32_t time)
{
    if (m_dragIcon) {
        m_dragIcon->setRequestedPosition(m_pointer->currentPosition());
    }

    if (m_dragFocusResource && m_dragFocus) {
        const QPointF &surfacePoint = m_compositor->waylandCompositor()->mapToView(m_dragFocus, m_pointer->currentPosition());
        qDebug() << Q_FUNC_INFO << m_pointer->currentPosition() << surfacePoint;
        send_motion(m_dragFocusResource->handle, time,
                    wl_fixed_from_double(surfacePoint.x()), wl_fixed_from_double(surfacePoint.y()));
    }
}

void DataDevice::button(uint32_t time, Qt::MouseButton button, uint32_t state)
{
    Q_UNUSED(time);

    if (m_dragFocusResource &&
        m_pointer->grabButton() == button &&
        state == Pointer::button_state_released)
        send_drop(m_dragFocusResource->handle);

    if (!m_pointer->buttonPressed() &&
        state == Pointer::button_state_released) {

        if (m_dragIcon) {
            m_dragIcon = 0;
            Q_EMIT m_inputDevice->dragHandle()->iconChanged();
        }

        setDragFocus(0, QPointF());
        m_pointer->endGrab();
    }
}

void DataDevice::data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial)
{
    if (m_inputDevice->pointerDevice()->grabSerial() == serial) {
        if (!m_inputDevice->pointerDevice()->buttonPressed() ||
             m_inputDevice->pointerDevice()->focusSurface()->surface()->handle() != Surface::fromResource(origin))
            return;

        m_dragClient = resource->client();
        m_dragDataSource = source != 0 ? DataSource::fromResource(source) : 0;
        m_dragIcon = icon != 0 ? m_compositor->waylandCompositor()->createView(Surface::fromResource(icon)->waylandSurface()) : 0;
        Q_EMIT m_inputDevice->dragHandle()->iconChanged();

        m_inputDevice->pointerDevice()->setFocus(0, QPointF());
        m_inputDevice->pointerDevice()->startGrab(this);
    }
}

void DataDevice::data_device_set_selection(Resource *, struct ::wl_resource *source, uint32_t serial)
{
    Q_UNUSED(serial);

    DataSource *dataSource = source ? DataSource::fromResource(source) : 0;

    if (m_selectionSource)
        m_selectionSource->cancel();

    m_selectionSource = dataSource;
    m_compositor->dataDeviceManager()->setCurrentSelectionSource(m_selectionSource);
    if (m_selectionSource)
        m_selectionSource->setDevice(this);

    QtWaylandServer::wl_keyboard::Resource *focusResource = m_inputDevice->keyboardDevice()->focusResource();
    Resource *resource = focusResource ? resourceMap().value(focusResource->client()) : 0;

    if (resource && m_selectionSource) {
        DataOffer *offer = new DataOffer(m_selectionSource, resource);
        send_selection(resource->handle, offer->resource()->handle);
    } else if (resource) {
        send_selection(resource->handle, 0);
    }
}

}

QT_END_NAMESPACE
