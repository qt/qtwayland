/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylanddatadevicemanager.h"

#include "qwaylandinputdevice.h"
#include "qwaylanddataoffer.h"
#include "qwaylanddatasource.h"
#include "qwaylandshmbackingstore.h"

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <QtGui/QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformclipboard.h>
#include <QtGui/QPainter>

#include <qpa/qwindowsysteminterface.h>

#include <QtCore/QDebug>

QT_USE_NAMESPACE

void QWaylandDataDeviceManager::data_offer(void *data,
                   struct wl_data_device *data_device,
                   struct wl_data_offer *id)
{
    QWaylandDataDeviceManager *handler = static_cast<QWaylandDataDeviceManager *>(data);

    new QWaylandDataOffer(handler->display(),id);
}

void QWaylandDataDeviceManager::enter(void *data,
              struct wl_data_device *wl_data_device,
              uint32_t time,
              struct wl_surface *surface,
              wl_fixed_t x,
              wl_fixed_t y,
              struct wl_data_offer *id)
{

    Q_UNUSED(wl_data_device);
    Q_UNUSED(x);
    Q_UNUSED(y);
    QWaylandDataDeviceManager *data_device_manager = static_cast<QWaylandDataDeviceManager *>(data);
    if (time < data_device_manager->m_drag_last_event_time)
        return;
    data_device_manager->m_drag_last_event_time = time;

    data_device_manager->m_drag_current_event_window = QWaylandWindow::fromWlSurface(surface);
    if (!surface)
        return;
    QWaylandDataOffer *offer = static_cast<QWaylandDataOffer *>(wl_data_offer_get_user_data(id));
    if (!offer)
        return;

//    QPoint point(x,y);
//    Qt::DropActions allActions =  Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
//    QPlatformDragQtResponse response = QWindowSystemInterface::handleDrag(data_device_manager->m_drag_current_event_window->window(),
//                                                                          offer,point,allActions);
//    data_device_manager->m_drag_can_drop = response.accepted();
    data_device_manager->m_drag_data_offer = offer;
    if (data_device_manager->m_drag_can_drop) {
        const char *first_offer = qPrintable(offer->formats_sys().at(0));
        wl_data_offer_accept(offer->handle(),QWaylandDisplay::currentTimeMillisec(),first_offer);
    } else {
        wl_data_offer_accept(offer->handle(),QWaylandDisplay::currentTimeMillisec(),0);
    }
}

void QWaylandDataDeviceManager::leave(void *data,
              struct wl_data_device *wl_data_device)
{
    Q_UNUSED(wl_data_device);
    QWaylandDataDeviceManager *data_device_manager = static_cast<QWaylandDataDeviceManager *>(data);
//    QWindowSystemInterface::handleDrag(data_device_manager->m_drag_current_event_window->window(),0,QPoint(0,0),Qt::IgnoreAction);
    data_device_manager->m_drag_can_drop = false;
    data_device_manager->m_drag_data_offer = 0;
    data_device_manager->m_drag_last_event_time = 0;
    data_device_manager->m_drag_current_event_window = 0;
    data_device_manager->m_drag_position = QPoint();
}

void QWaylandDataDeviceManager::motion(void *data,
               struct wl_data_device *wl_data_device,
               uint32_t time,
               wl_fixed_t x,
               wl_fixed_t y)
{
    Q_UNUSED(wl_data_device);
    QWaylandDataDeviceManager *data_device_manager = static_cast<QWaylandDataDeviceManager *>(data);
    if (time < data_device_manager->m_drag_last_event_time)
        return;
    data_device_manager->m_drag_position = QPoint(wl_fixed_to_int(x), wl_fixed_to_int(y));

//    Qt::DropActions allActions =  Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
//    QPlatformDragQtResponse response = QWindowSystemInterface::handleDrag(data_device_manager->m_drag_current_event_window->window(),
//                                       data_device_manager->m_drag_data_offer, data_device_manager->m_drag_position ,allActions);
//    data_device_manager->m_drag_can_drop = response.accepted();
    const char *offerMime = 0;
    if (data_device_manager->m_drag_can_drop) {
        offerMime = qPrintable(data_device_manager->m_drag_data_offer->formats_sys().at(0));
    }
    wl_data_offer_accept(data_device_manager->m_drag_data_offer->handle(),QWaylandDisplay::currentTimeMillisec(),offerMime);
}

void QWaylandDataDeviceManager::drop(void *data,
             struct wl_data_device *wl_data_device)
{
    Q_UNUSED(wl_data_device);
    QWaylandDataDeviceManager *data_device_manager = static_cast<QWaylandDataDeviceManager *>(data);
    QWindow *window = data_device_manager->m_drag_current_event_window->window();
    QMimeData *mime = data_device_manager->m_drag_data_offer;
//    QPoint point = data_device_manager->m_drag_position;
//    Qt::DropActions allActions =  Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;

    Q_UNUSED(window);
    Q_UNUSED(mime);
//    QWindowSystemInterface::handleDrop(window,mime,point,allActions);
}


void QWaylandDataDeviceManager::selection(void *data,
                                            struct wl_data_device *wl_data_device,
                                            struct wl_data_offer *id)
{
    Q_UNUSED(wl_data_device);

    QWaylandDataDeviceManager *handler = static_cast<QWaylandDataDeviceManager *>(data);
    QWaylandDataOffer *mime = handler->m_selection_data_offer;
    delete mime;
    QWaylandDataSource *transfer_source = handler->m_selection_data_source;
    delete transfer_source;
    handler->m_selection_data_source = 0;

    if (id) {
        mime = static_cast<QWaylandDataOffer *>(wl_data_offer_get_user_data(id));
        handler->m_selection_data_offer = mime;
    } else {
        handler->m_selection_data_offer = 0;
    }

    QGuiApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Clipboard);
}

const struct wl_data_device_listener QWaylandDataDeviceManager::transfer_device_listener = {
    QWaylandDataDeviceManager::data_offer,
    QWaylandDataDeviceManager::enter,
    QWaylandDataDeviceManager::leave,
    QWaylandDataDeviceManager::motion,
    QWaylandDataDeviceManager::drop,
    QWaylandDataDeviceManager::selection
};

QWaylandDataDeviceManager::QWaylandDataDeviceManager(QWaylandDisplay *display, uint32_t id)
    : m_display(display)
    , m_selection_data_offer(0)
    , m_selection_data_source(0)
    , m_drag_data_offer(0)
    , m_drag_data_source(0)
    , m_drag_surface(0)
    , m_drag_icon_surface(0)
    , m_drag_icon_buffer(0)
    , m_drag_can_drop(false)
{
    m_data_device_manager = static_cast<struct wl_data_device_manager *>(wl_registry_bind(display->wl_registry(),id,&wl_data_device_manager_interface,1));

    // Create transfer devices for all input devices.
    // ### This only works if we get the global before all devices and is surely wrong when hotplugging.
    QList<QWaylandInputDevice *> inputDevices = m_display->inputDevices();
    for (int i = 0; i < inputDevices.size();i++) {
        inputDevices.at(i)->setTransferDevice(getDataDevice(inputDevices.at(i)));
    }
}

QWaylandDataDeviceManager::~QWaylandDataDeviceManager()
{
    wl_data_device_manager_destroy(m_data_device_manager);
}

struct wl_data_device *QWaylandDataDeviceManager::getDataDevice(QWaylandInputDevice *inputDevice)
{
    struct wl_data_device *transfer_device = wl_data_device_manager_get_data_device(m_data_device_manager,
                                                                                    inputDevice->wl_seat());
    wl_data_device_add_listener(transfer_device,&transfer_device_listener,this);

    return transfer_device;
}

QWaylandDataOffer *QWaylandDataDeviceManager::selectionTransfer() const
{
    return m_selection_data_offer;
}

void QWaylandDataDeviceManager::createAndSetDrag(QDrag *drag)
{
    QWaylandInputDevice *inputDevice = m_display->lastKeyboardFocusInputDevice();
    if (!inputDevice)
        return;

    if (m_drag_data_source) {
        qDebug() << "QWaylandDndSelectionHandler::createAndSetDrag: Allready have a valid drag";
        delete m_drag_data_source;
    }

    delete m_drag_data_offer;
    m_drag_data_offer = 0;

    m_drag_data_source = new QWaylandDataSource(this,drag->mimeData());

    struct wl_data_device *transfer_device = inputDevice->transferDevice();
    m_drag_surface = m_display->createSurface(this);
    QPixmap pixmap = drag->pixmap();
    if (pixmap.isNull()) {
//        pixmap = QPlatformDrag::defaultPixmap();
    }

    m_drag_icon_buffer = new QWaylandShmBuffer(m_display, pixmap.size(), QImage::Format_ARGB32_Premultiplied);

    {
        QPainter p(m_drag_icon_buffer->image());
        p.drawPixmap(0,0,pixmap);
    }

    m_drag_icon_surface = m_display->compositor()->create_surface();
    wl_surface_attach(m_drag_icon_surface, m_drag_icon_buffer->buffer(), -drag->hotSpot().x(), -drag->hotSpot().y());

    wl_data_device_start_drag(transfer_device, m_drag_data_source->handle(), m_drag_surface, m_drag_icon_surface, QWaylandDisplay::currentTimeMillisec());
}

QMimeData *QWaylandDataDeviceManager::dragMime() const
{
    if (m_drag_data_offer) {
        return m_drag_data_offer;
    } else if (m_drag_data_source){
        return m_drag_data_source->mimeData();
    }
    return 0;
}

bool QWaylandDataDeviceManager::canDropDrag() const
{
    return m_drag_can_drop;
}

void QWaylandDataDeviceManager::cancelDrag()
{
    wl_data_source_destroy(m_drag_data_source->handle() );
    wl_surface_destroy(m_drag_icon_surface);
    m_drag_data_source = 0;
}

void QWaylandDataDeviceManager::createAndSetSelectionSource(QMimeData *mimeData, QClipboard::Mode mode)
{
    Q_UNUSED(mode);
    QWaylandDataSource *transfer_source = m_selection_data_source;
    delete transfer_source;

    transfer_source = new QWaylandDataSource(this,mimeData);
    m_selection_data_source = transfer_source;
    QWaylandInputDevice *inputDevice = m_display->lastKeyboardFocusInputDevice();
    if (!inputDevice)
        inputDevice = m_display->inputDevices().first(); // try to survive in apps without any surfaces
    struct wl_data_device *transfer_device = inputDevice->transferDevice();
    wl_data_device_set_selection(transfer_device,transfer_source->handle(),QWaylandDisplay::currentTimeMillisec());

    QGuiApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Clipboard);
}

QWaylandDataSource *QWaylandDataDeviceManager::selectionTransferSource()
{
    return m_selection_data_source;
}


QWaylandDisplay * QWaylandDataDeviceManager::display() const
{
    return m_display;
}

struct wl_data_device_manager *QWaylandDataDeviceManager::handle() const
{
    return m_data_device_manager;
}


