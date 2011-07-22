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

#ifndef WLDRAG_H
#define WLDRAG_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <wayland-server.h>

QT_BEGIN_NAMESPACE
class QSocketNotifier;
QT_END_NAMESPACE

namespace Wayland {

class Surface;

class Drag : public QObject
{
    Q_OBJECT

public:
    static Drag *instance();
    Drag();
    void create(struct wl_client *client, uint32_t id);
    void dragMove(const QPoint &global, const QPoint &local, Surface *surface);
    void dragEnd();

private:
    static void destroyDrag(struct wl_resource *resource, struct wl_client *client);

    static void dragOffer(struct wl_client *client, struct wl_drag *drag, const char *type);
    static void dragActivate(struct wl_client *client,
                             struct wl_drag *drag,
                             struct wl_surface *surface,
                             struct wl_input_device *device, uint32_t time);
    static void dragDestroy(struct wl_client *client, struct wl_drag *drag);
    static const struct wl_drag_interface dragInterface;

    static void dragOfferAccept(struct wl_client *client,
                                struct wl_drag_offer *offer, uint32_t time, const char *type);
    static void dragOfferReceive(struct wl_client *client,
                                 struct wl_drag_offer *offer, int fd);
    static void dragOfferReject(struct wl_client *client, struct wl_drag_offer *offer);
    static const struct wl_drag_offer_interface dragOfferInterface;

    void setPointerFocus(wl_surface *surface, const QPoint &global, const QPoint &local);
    void done(bool sending);

    QStringList m_offerList;
    wl_drag *m_drag;
};

}

#endif // WLDRAG_H
