/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandprimaryselectionv1_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandmimehelper_p.h"

#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qplatformclipboard.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandPrimarySelectionDeviceManagerV1::QWaylandPrimarySelectionDeviceManagerV1(QWaylandDisplay *display, uint id, uint version)
    : zwp_primary_selection_device_manager_v1(display->wl_registry(), id, qMin(version, uint(1)))
    , m_display(display)
{
    // Create devices for all seats.
    // This only works if we get the global before all devices
    const auto seats = m_display->inputDevices();
    for (auto *seat : seats)
        seat->setPrimarySelectionDevice(createDevice(seat));
}

QWaylandPrimarySelectionDeviceV1 *QWaylandPrimarySelectionDeviceManagerV1::createDevice(QWaylandInputDevice *seat)
{
    return new QWaylandPrimarySelectionDeviceV1(this, seat);
}

QWaylandPrimarySelectionOfferV1::QWaylandPrimarySelectionOfferV1(QWaylandDisplay *display, ::zwp_primary_selection_offer_v1 *offer)
    : zwp_primary_selection_offer_v1(offer)
    , m_display(display)
    , m_mimeData(new QWaylandMimeData(this))
{}

void QWaylandPrimarySelectionOfferV1::startReceiving(const QString &mimeType, int fd)
{
    receive(mimeType, fd);
    wl_display_flush(m_display->wl_display());
}

void QWaylandPrimarySelectionOfferV1::zwp_primary_selection_offer_v1_offer(const QString &mime_type)
{
    m_mimeData->appendFormat(mime_type);
}

QWaylandPrimarySelectionDeviceV1::QWaylandPrimarySelectionDeviceV1(
        QWaylandPrimarySelectionDeviceManagerV1 *manager, QWaylandInputDevice *seat)
    : QtWayland::zwp_primary_selection_device_v1(manager->get_device(seat->wl_seat()))
    , m_display(manager->display())
    , m_seat(seat)
{
}

QWaylandPrimarySelectionDeviceV1::~QWaylandPrimarySelectionDeviceV1()
{
    destroy();
}

void QWaylandPrimarySelectionDeviceV1::invalidateSelectionOffer()
{
    if (!m_selectionOffer)
        return;

    m_selectionOffer.reset();
    QGuiApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Selection);
}

void QWaylandPrimarySelectionDeviceV1::setSelectionSource(QWaylandPrimarySelectionSourceV1 *source)
{
    if (source) {
        connect(source, &QWaylandPrimarySelectionSourceV1::cancelled, this, [this]() {
            m_selectionSource.reset();
            QGuiApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Selection);
        });
    }
    set_selection(source ? source->object() : nullptr, m_seat->serial());
    m_selectionSource.reset(source);
}

void QWaylandPrimarySelectionDeviceV1::zwp_primary_selection_device_v1_data_offer(zwp_primary_selection_offer_v1 *offer)
{
    new QWaylandPrimarySelectionOfferV1(m_display, offer);
}

void QWaylandPrimarySelectionDeviceV1::zwp_primary_selection_device_v1_selection(zwp_primary_selection_offer_v1 *id)
{

    if (id)
        m_selectionOffer.reset(static_cast<QWaylandPrimarySelectionOfferV1 *>(zwp_primary_selection_offer_v1_get_user_data(id)));
    else
        m_selectionOffer.reset();

    QGuiApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Selection);
}

QWaylandPrimarySelectionSourceV1::QWaylandPrimarySelectionSourceV1(QWaylandPrimarySelectionDeviceManagerV1 *manager, QMimeData *mimeData)
    : QtWayland::zwp_primary_selection_source_v1(manager->create_source())
    , m_mimeData(mimeData)
{
    if (!mimeData)
        return;
    for (auto &format : mimeData->formats())
        offer(format);
}

QWaylandPrimarySelectionSourceV1::~QWaylandPrimarySelectionSourceV1()
{
    destroy();
}

void QWaylandPrimarySelectionSourceV1::zwp_primary_selection_source_v1_send(const QString &mime_type, int32_t fd)
{
    QByteArray content = QWaylandMimeHelper::getByteArray(m_mimeData, mime_type);
    if (!content.isEmpty()) {
        // Create a sigpipe handler that does nothing, or clients may be forced to terminate
        // if the pipe is closed in the other end.
        struct sigaction action, oldAction;
        action.sa_handler = SIG_IGN;
        sigemptyset (&action.sa_mask);
        action.sa_flags = 0;

        sigaction(SIGPIPE, &action, &oldAction);
        write(fd, content.constData(), size_t(content.size()));
        sigaction(SIGPIPE, &oldAction, nullptr);
    }
    close(fd);
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
