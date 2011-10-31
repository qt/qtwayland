/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "wldrag.h"
#include "wlcompositor.h"
#include "wlsurface.h"
#include <wayland-util.h>
#include <string.h>
#include <unistd.h>
#include <QDebug>

namespace Wayland {

void Drag::dragOfferAccept(struct wl_client *client,
                           struct wl_resource *resource_offer, uint32_t time, const char *type)
{
    Q_UNUSED(time);
    Drag *self = Drag::instance();
    struct wl_drag_offer *offer = reinterpret_cast<struct wl_drag_offer *>(resource_offer);
    struct wl_drag *drag = container_of(offer, struct wl_drag, drag_offer);

    qDebug() << "dragOfferAccept" << client << offer << type;
    drag->target = client;
    QString wantedType = QString::fromLatin1(type);
    if (!self->m_offerList.contains(wantedType)) {
        qWarning("dragOfferAccept: Client accepted type '%s' that has not been offered",
                 qPrintable(type));
        type = 0;
    }
    wl_resource_post_event(&drag->resource,
                         WL_DRAG_TARGET, type);
}

void Drag::dragOfferReceive(struct wl_client *client,
                            struct wl_resource *resource_offer, int fd)
{
    Q_UNUSED(client);
    struct wl_drag_offer *offer = reinterpret_cast<struct wl_drag_offer *>(resource_offer);

    qDebug() << "dragOfferReceive" << client << offer << fd;
    struct wl_drag *drag = container_of(offer, struct wl_drag, drag_offer);
    wl_resource_post_event(&drag->resource,
                         WL_DRAG_FINISH, fd);
    close(fd);
}

void Drag::dragOfferReject(struct wl_client *client, struct wl_resource *resource_offer)
{
    Q_UNUSED(client);
    struct wl_drag_offer *offer = reinterpret_cast<struct wl_drag_offer *>(resource_offer);
    qDebug() << "dragOfferReject" << client << offer;

    struct wl_drag *drag = container_of(offer, struct wl_drag, drag_offer);
    if (drag->target == client)
        drag->target = 0;
    wl_resource_post_event(&drag->resource,
                         WL_DRAG_REJECT);
}

const struct wl_drag_offer_interface Drag::dragOfferInterface = {
    Drag::dragOfferAccept,
    Drag::dragOfferReceive,
    Drag::dragOfferReject
};

void Drag::dragOffer(struct wl_client *client, struct wl_resource *drag, const char *type)
{
    qDebug() << "dragOffer" << client << drag << type;
    Q_UNUSED(client);
    Q_UNUSED(drag);
    instance()->m_offerList.append(QString::fromLatin1(type));
}

void Drag::dragActivate(struct wl_client *client,
                        struct wl_resource *drag_resource,
                        struct wl_resource *surface_resource,
                        struct wl_resource *device_resource, uint32_t time)
{
    struct wl_drag *drag = reinterpret_cast<struct wl_drag *>(drag_resource);
    struct wl_surface *surface = reinterpret_cast<struct wl_surface *>(surface_resource);
    struct wl_input_device *device = reinterpret_cast<struct wl_input_device *>(device_resource);

    qDebug() << "dragActivate" << client << drag << surface;
    Q_UNUSED(client);
    Q_UNUSED(device);
    Q_UNUSED(time);
    Drag *self = Drag::instance();
    drag->source = surface;
    drag->drag_offer.resource.object.interface = &wl_drag_offer_interface;
    drag->drag_offer.resource.object.implementation = (void (**)()) &dragOfferInterface;
    wl_display *dpy = Compositor::instance()->wl_display();
//    wl_display_add_object(dpy, &drag->drag_offer.resource.object);
//    wl_display_add_global(dpy, &drag->drag_offer.resource.object, 0);
    Surface *focus = Compositor::instance()->pointerFocus();
    QPoint pos;
    if (focus)
        pos = focus->lastMousePos();
    // ### Sending local as global, which is wrong, but oh well.
    self->setPointerFocus(surface, pos, pos);
}

void Drag::setPointerFocus(wl_surface *surface, const QPoint &global, const QPoint &local)
{
    if (!m_drag)
        return;

    if (m_drag->drag_focus == surface)
        return;

    uint timestamp = Compositor::currentTimeMsecs();
    if (m_drag->drag_focus
        && (!surface || m_drag->drag_focus->resource.client != surface->resource.client)) {
        qDebug() << "WL_DRAG_OFFER_POINTER_FOCUS with null";
        wl_resource_post_event(&m_drag->drag_offer.resource,
                             WL_DRAG_OFFER_POINTER_FOCUS,
                             timestamp, 0, 0, 0, 0, 0);
    }
    if (surface
        && (!m_drag->drag_focus || m_drag->drag_focus->resource.client != surface->resource.client)) {
//        wl_client_post_global(surface->client,
//                              &m_drag->drag_offer.object);
        foreach (const QString &format, m_offerList) {
            QByteArray mimeTypeBa = format.toLatin1();
            qDebug() << "WL_DRAG_OFFER_OFFER" << mimeTypeBa;
            wl_resource_post_event(&m_drag->drag_offer.resource,
                                 WL_DRAG_OFFER_OFFER, mimeTypeBa.constData());
        }
    }

    if (surface) {
        qDebug() << "WL_DRAG_OFFER_POINTER_FOCUS" << surface << global << local;
        wl_resource_post_event(&m_drag->drag_offer.resource,
                             WL_DRAG_OFFER_POINTER_FOCUS,
                             timestamp, surface,
                             global.x(), global.y(), local.x(), local.y());
        Compositor::instance()->m_dragActive = true;
    }

    m_drag->drag_focus = surface;
    m_drag->target = 0;
}

void Drag::dragDestroy(struct wl_client *client, struct wl_resource *drag_resource)
{
    Q_UNUSED(client);
    qDebug() << "dragDestroy";
    struct wl_drag *drag = reinterpret_cast<struct wl_drag *>(drag_resource);
    wl_resource_destroy(&drag->resource, Compositor::currentTimeMsecs());
}

const struct wl_drag_interface Drag::dragInterface = {
    Drag::dragOffer,
    Drag::dragActivate,
    Drag::dragDestroy
};

void Drag::destroyDrag(struct wl_resource *resource)
{
    struct wl_drag *drag = container_of(resource, struct wl_drag, resource);
    wl_display *dpy = Compositor::instance()->wl_display();
//    wl_display_remove_global(dpy, &drag->drag_offer.object);
    delete drag;
}

void Drag::create(struct wl_client *client, uint32_t id)
{
    Q_UNUSED(client);
    m_offerList.clear();
    wl_drag *drag = new wl_drag;
    memset(drag, 0, sizeof *drag);
    drag->resource.object.id = id;
    drag->resource.object.interface = &wl_drag_interface;
    drag->resource.object.implementation = (void (**)()) &dragInterface;
    drag->resource.destroy = destroyDrag;
    wl_client_add_resource(client, &drag->resource);
    m_drag = drag;
}

void Drag::done(bool sending)
{
    qDebug() << "drag done";
    Compositor::instance()->m_dragActive = false;
    if (!sending) {
        setPointerFocus(0, QPoint(), QPoint());
        // ### hack: Send a pointerFocus with null surface to the source too, this is
        // mandatory even if the previous pointerFocus went to the same client, otherwise
        // Qt will not know the drag is over without a drop.
        wl_resource_post_event(&m_drag->drag_offer.resource,
                             WL_DRAG_OFFER_POINTER_FOCUS,
                             Compositor::instance()->currentTimeMsecs(),
                             0, 0, 0, 0, 0);
    }
    m_drag = 0;
}

void Drag::dragMove(const QPoint &global, const QPoint &local, Surface *surface)
{
    if (!m_drag)
        return;
//    qDebug() << "dragMove" << global << local << surface;
    if (surface) {
        setPointerFocus(surface->base(), global, local);
        uint timestamp = Compositor::currentTimeMsecs();
        wl_resource_post_event(&m_drag->drag_offer.resource,
                             WL_DRAG_OFFER_MOTION,
                             timestamp,
                             global.x(), global.y(), local.x(), local.y());
    } else {
        setPointerFocus(0, global, local);
    }
}

void Drag::dragEnd()
{
    qDebug() << "dragEnd";
    if (!m_drag)
        return;
    if (m_drag->target) {
        qDebug() << "WL_DRAG_OFFER_DROP" << m_drag->target;
        wl_resource_post_event(&m_drag->drag_offer.resource,
                             WL_DRAG_OFFER_DROP);
        done(true);
    } else {
        done(false);
    }
}

Q_GLOBAL_STATIC(Drag, globalInstance)

Drag *Drag::instance()
{
    return globalInstance();
}

Drag::Drag()
    : m_drag(0)
{
}

}
