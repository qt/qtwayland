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

#include "wlselection.h"
#include "wlcompositor.h"
#include <wayland-util.h>
#include <string.h>
#include <unistd.h>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>

namespace Wayland {

void Selection::send(struct wl_client *client,
                     struct wl_selection_offer *offer,
                     const char *mime_type, int fd)
{
    Q_UNUSED(client);
    Selection *self = instance();
    if (self->m_retainedSelection) {
        QByteArray data = self->m_retainedData.data(QString::fromLatin1(mime_type));
        if (!data.isEmpty()) {
            QFile f;
            if (f.open(fd, QIODevice::WriteOnly))
                f.write(data);
        }
    } else {
        struct wl_selection *selection = container_of(offer, struct wl_selection, selection_offer);
        wl_client_post_event(selection->client,
                             &selection->resource.object,
                             WL_SELECTION_SEND, mime_type, fd);
    }
    close(fd);
}

const struct wl_selection_offer_interface Selection::selectionOfferInterface = {
    Selection::send
};

void Selection::selOffer(struct wl_client *client,
                         struct wl_selection *selection,
                         const char *type)
{
    Q_UNUSED(client);
    Q_UNUSED(selection);
    instance()->m_offerList.append(QString::fromLatin1(type));
}

void Selection::selActivate(struct wl_client *client,
                            struct wl_selection *selection,
                            struct wl_input_device *device,
                            uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(device);
    Q_UNUSED(time);
    Selection *self = Selection::instance();

    selection->selection_offer.object.interface = &wl_selection_offer_interface;
    selection->selection_offer.object.implementation = (void (**)()) &selectionOfferInterface;
    wl_display_add_object(Compositor::instance()->wl_display(), &selection->selection_offer.object);

    QList<struct wl_client *> clients = Compositor::instance()->clients();
    if (self->m_currentSelection) {
        if (!clients.contains(self->m_currentSelection->client))
            self->m_currentSelection = 0;
        else
            wl_client_post_event(self->m_currentSelection->client,
                                 &self->m_currentSelection->resource.object,
                                 WL_SELECTION_CANCELLED);
    }
    self->m_currentSelection = selection;

    if (self->m_currentOffer) {
        foreach (struct wl_client *client, clients) {
            wl_client_post_event(client, &self->m_currentOffer->object,
                                 WL_SELECTION_OFFER_KEYBOARD_FOCUS, 0);
        }
    }
    self->m_currentOffer = &selection->selection_offer;
    foreach (struct wl_client *client, clients) {
        wl_client_post_global(client, &selection->selection_offer.object);
    }
    foreach (struct wl_client *client, clients) {
        foreach (const QString &mimeType, self->m_offerList) {
            QByteArray mimeTypeBa = mimeType.toLatin1();
            wl_client_post_event(client, &selection->selection_offer.object,
                                 WL_SELECTION_OFFER_OFFER, mimeTypeBa.constData());
        }
    }
    foreach (struct wl_client *client, clients) {
        wl_client_post_event(client, &selection->selection_offer.object,
                             WL_SELECTION_OFFER_KEYBOARD_FOCUS, selection->input_device);
    }

    if (self->m_retainedSelectionEnabled) {
        self->m_retainedData.clear();
        self->m_retainedReadIndex = 0;
        self->retain();
    }
}

void Selection::retain()
{
    finishReadFromClient();
    if (m_retainedReadIndex >= m_offerList.count()) {
        if (m_watchFunc)
            m_watchFunc(&m_retainedData, m_watchFuncParam);
        return;
    }
    QString mimeType = m_offerList.at(m_retainedReadIndex);
    m_retainedReadBuf.clear();
    QByteArray mimeTypeBa = mimeType.toLatin1();
    int fd[2];
    if (pipe(fd) == -1) {
        qWarning("Clipboard: Failed to create pipe");
        return;
    }
    wl_client_post_event(m_currentSelection->client, &m_currentSelection->resource.object,
                         WL_SELECTION_SEND, mimeTypeBa.constData(), fd[1]);
    close(fd[1]);
    m_retainedReadNotifier = new QSocketNotifier(fd[0], QSocketNotifier::Read, this);
    connect(m_retainedReadNotifier, SIGNAL(activated(int)), SLOT(readFromClient(int)));
}

void Selection::finishReadFromClient()
{
    if (m_retainedReadNotifier) {
        int fd = m_retainedReadNotifier->socket();
        delete m_retainedReadNotifier;
        m_retainedReadNotifier = 0;
        close(fd);
    }
}

void Selection::readFromClient(int fd)
{
    char buf[256];
    int n = read(fd, buf, sizeof buf);
    if (n <= 0) {
        finishReadFromClient();
        QString mimeType = m_offerList.at(m_retainedReadIndex);
        m_retainedData.setData(mimeType, m_retainedReadBuf);
        ++m_retainedReadIndex;
        retain();
    } else {
        m_retainedReadBuf.append(buf, n);
    }
}

void Selection::selDestroy(struct wl_client *client, struct wl_selection *selection)
{
    wl_resource_destroy(&selection->resource, client, Compositor::currentTimeMsecs());
}

const struct wl_selection_interface Selection::selectionInterface = {
    Selection::selOffer,
    Selection::selActivate,
    Selection::selDestroy
};

void Selection::destroySelection(struct wl_resource *resource, struct wl_client *client)
{
    Q_UNUSED(client);
    struct wl_selection *selection = container_of(resource, struct wl_selection, resource);
    Selection *self = Selection::instance();
    if (self->m_currentSelection == selection)
        self->m_currentSelection = 0;
    if (self->m_currentOffer == &selection->selection_offer) {
        self->m_currentOffer = 0;
        if (self->m_retainedSelectionEnabled) {
            delete self->m_retainedSelection;
            self->m_retainedSelection = selection;
            return;
        }
        self->m_offerList.clear();
        foreach (struct wl_client *client, Compositor::instance()->clients())
            wl_client_post_event(client, &selection->selection_offer.object,
                                 WL_SELECTION_OFFER_KEYBOARD_FOCUS, 0);
    }
    delete selection;
}

void Selection::create(struct wl_client *client, uint32_t id)
{
    delete m_retainedSelection;
    m_retainedSelection = 0;
    m_offerList.clear();
    struct wl_selection *selection = new struct wl_selection;
    memset(selection, 0, sizeof *selection);
    selection->resource.object.id = id;
    selection->resource.object.interface = &wl_selection_interface;
    selection->resource.object.implementation = (void (**)()) &selectionInterface;
    selection->resource.destroy = destroySelection;
    selection->client = client;
    selection->input_device = Compositor::instance()->inputDevice();
    wl_client_add_resource(client, &selection->resource);
}

void Selection::setRetainedSelection(bool enable)
{
    m_retainedSelectionEnabled = enable;
}

void Selection::setRetainedSelectionWatcher(Watcher func, void *param)
{
    m_watchFunc = func;
    m_watchFuncParam = param;
}

void Selection::onClientAdded(wl_client *client)
{
    struct wl_selection *selection = m_currentSelection;
    struct wl_selection_offer *offer = m_currentOffer;
    if (m_retainedSelection) {
        selection = m_retainedSelection;
        offer = &m_retainedSelection->selection_offer;
    }
    if (selection && offer) {
        wl_client_post_global(client, &offer->object);
        foreach (const QString &mimeType, m_offerList) {
            QByteArray mimeTypeBa = mimeType.toLatin1();
            wl_client_post_event(client, &offer->object,
                                 WL_SELECTION_OFFER_OFFER, mimeTypeBa.constData());
        }
        wl_client_post_event(client, &offer->object,
                             WL_SELECTION_OFFER_KEYBOARD_FOCUS, selection->input_device);
    }
}

Q_GLOBAL_STATIC(Selection, globalInstance)

Selection *Selection::instance()
{
    return globalInstance();
}

Selection::Selection()
    : m_currentSelection(0), m_currentOffer(0),
      m_retainedReadNotifier(0), m_retainedSelection(0),
      m_retainedSelectionEnabled(false),
      m_watchFunc(0), m_watchFuncParam(0)
{
    connect(Compositor::instance(), SIGNAL(clientAdded(wl_client*)), SLOT(onClientAdded(wl_client*)));
}

Selection::~Selection()
{
    finishReadFromClient();
}

}
