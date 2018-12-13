/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "datadevice.h"

namespace MockCompositor {

void DataDeviceManager::data_device_manager_get_data_device(Resource *resource, uint32_t id, wl_resource *seatResource)
{
    auto *seat = fromResource<Seat>(seatResource);
    QVERIFY(seat);
    QVERIFY(!seat->m_dataDevice);
    seat->m_dataDevice.reset(new DataDevice(resource->client(), id, resource->version()));
}

DataDevice::~DataDevice()
{
    // If the client hasn't deleted the wayland object, just ignore subsequent events
    if (auto *r = resource()->handle)
        wl_resource_set_implementation(r, nullptr, nullptr, nullptr);
}

void DataDevice::sendDataOffer(DataOffer *offer)
{
    wl_data_device::send_data_offer(offer->resource()->handle);
}

void DataDevice::sendSelection(DataOffer *offer)
{
    wl_data_device::send_selection(offer->resource()->handle);
}

void DataOffer::data_offer_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void DataOffer::data_offer_receive(Resource *resource, const QString &mime_type, int32_t fd)
{
    Q_UNUSED(resource);
    emit receive(mime_type, fd);
}

} // namespace MockCompositor
