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
#include "wlinputdevice.h"

#include <wayland-util.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>
#include <QtCore/private/qcore_unix_p.h>

#include <QtCore/QDebug>

namespace Wayland {

void Selection::send(struct wl_client *client,
                     struct wl_resource *offer,
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
        struct wl_selection_offer *actual_offer = reinterpret_cast<struct wl_selection_offer *>(offer);
        struct wl_selection *selection = container_of(actual_offer, struct wl_selection, selection_offer);
        wl_resource_post_event(&selection->resource,
                             WL_SELECTION_SEND, mime_type, fd);
    }
    close(fd);
}

const struct wl_selection_offer_interface Selection::selectionOfferInterface = {
    Selection::send
};

void Selection::selOffer(struct wl_client *client,
                         struct wl_resource *selection,
                         const char *type)
{
    Q_UNUSED(client);
    Q_UNUSED(selection);
    instance()->m_offerList.append(QString::fromLatin1(type));
}

void Selection::selActivate(struct wl_client *client,
                            struct wl_resource *selection_resource,
                            struct wl_resource *device,
                            uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(device);
    Q_UNUSED(time);
    Selection *self = Selection::instance();
    struct wl_selection *selection = reinterpret_cast<struct wl_selection *>(selection_resource);
    selection->selection_offer.resource.object.interface = &wl_selection_offer_interface;
    selection->selection_offer.resource.object.implementation = (void (**)()) &selectionOfferInterface;
    wl_display *dpy = Compositor::instance()->wl_display();
    wl_display_add_global(dpy,&wl_selection_offer_interface,selection,0);

    QList<struct wl_client *> clients = Compositor::instance()->clients();

    if (self->m_currentSelection) {
        if (!clients.contains(self->m_currentSelection->client))
            self->m_currentSelection = 0;
        else
            wl_resource_post_event(&self->m_currentSelection->resource,
                                 WL_SELECTION_CANCELLED);
    }
    self->m_currentSelection = selection;

    if (self->m_currentOffer) {
        foreach (struct wl_resource *clientResource, self->m_selectionClientResources) {
            wl_resource_post_event(clientResource,
                                 WL_SELECTION_OFFER_KEYBOARD_FOCUS, 0);
        }
        self->m_selectionClientResources.clear();
    }
    self->m_currentOffer = &selection->selection_offer;
    foreach (struct wl_client *client, clients) {
        qDebug() << "sending to clients";
        selection->selection_offer.resource.client = client;
        foreach (const QString &mimeType, self->m_offerList) {
            QByteArray mimeTypeBa = mimeType.toLatin1();
            wl_resource_post_event(&selection->selection_offer.resource,
                                 WL_SELECTION_OFFER_OFFER, mimeTypeBa.constData());
        }

        wl_resource_post_event(&selection->selection_offer.resource,
                             WL_SELECTION_OFFER_KEYBOARD_FOCUS, selection->input_device);
    }

    if (self->m_retainedSelectionEnabled && client) {
        self->m_retainedData.clear();
        self->m_retainedReadIndex = 0;
        self->retain();
    }
}

void Selection::retain()
{
    finishReadFromClient();
    if (!m_currentSelection->client) // do not retain data created via overrideSelection()
        return;
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
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL, 0) | O_NONBLOCK);
    wl_resource_post_event(&m_currentSelection->resource,
                         WL_SELECTION_SEND, mimeTypeBa.constData(), fd[1]);
    close(fd[1]);
    m_retainedReadNotifier = new QSocketNotifier(fd[0], QSocketNotifier::Read, this);
    connect(m_retainedReadNotifier, SIGNAL(activated(int)), SLOT(readFromClient(int)));
}

void Selection::finishReadFromClient(bool exhausted)
{
    if (m_retainedReadNotifier) {
        if (exhausted) {
            int fd = m_retainedReadNotifier->socket();
            delete m_retainedReadNotifier;
            close(fd);
        } else {
            // Do not close the handle or destroy the read notifier here
            // or else clients may SIGPIPE.
            m_obsoleteRetainedReadNotifiers.append(m_retainedReadNotifier);
        }
        m_retainedReadNotifier = 0;
    }
}

void Selection::readFromClient(int fd)
{
    static char buf[4096];
    int obsCount = m_obsoleteRetainedReadNotifiers.count();
    for (int i = 0; i < obsCount; ++i) {
        QSocketNotifier *sn = m_obsoleteRetainedReadNotifiers.at(i);
        if (sn->socket() == fd) {
            // Read and drop the data, stopping to read and closing the handle
            // is not yet safe because that could kill the client with SIGPIPE
            // when it still tries to write.
            int n;
            do {
                n = QT_READ(fd, buf, sizeof buf);
            } while (n > 0);
            if (n != -1 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                m_obsoleteRetainedReadNotifiers.removeAt(i);
                delete sn;
                close(fd);
            }
            return;
        }
    }
    int n = QT_READ(fd, buf, sizeof buf);
    if (n <= 0) {
        if (n != -1 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            finishReadFromClient(true);
            QString mimeType = m_offerList.at(m_retainedReadIndex);
            m_retainedData.setData(mimeType, m_retainedReadBuf);
            ++m_retainedReadIndex;
            retain();
        }
    } else {
        m_retainedReadBuf.append(buf, n);
    }
}

void Selection::selDestroy(struct wl_client *client, struct wl_resource *selection)
{
    wl_resource_destroy(selection, Compositor::currentTimeMsecs());
}

const struct wl_selection_interface Selection::selectionInterface = {
    Selection::selOffer,
    Selection::selActivate,
    Selection::selDestroy
};

void Selection::destroySelection(struct wl_resource *resource)
{
    struct wl_selection *selection = container_of(resource, struct wl_selection, resource);
    Selection *self = Selection::instance();
    wl_display *dpy = Compositor::instance()->wl_display();
    if (self->m_currentSelection == selection)
        self->m_currentSelection = 0;
    if (self->m_currentOffer == &selection->selection_offer) {
        self->m_currentOffer = 0;
        if (self->m_retainedSelectionEnabled) {
            if (self->m_retainedSelection) {
//                wl_display_remove_global(dpy, &self->m_retainedSelection->selection_offer.object);
                delete self->m_retainedSelection;
            }
            self->m_retainedSelection = selection;
            return;
        }
        self->m_offerList.clear();
        foreach (struct wl_resource *resource, self->m_selectionClientResources)
            wl_resource_post_event(resource,
                                 WL_SELECTION_OFFER_KEYBOARD_FOCUS, 0);
    }
//    wl_display_remove_global(dpy, &selection->selection_offer.object);
    delete selection;
}

void Selection::create(struct wl_client *client, uint32_t id)
{
    if (m_retainedSelection) {
        if (m_retainedSelection == m_currentSelection) { // this can happen only when overrideSelection() was called
            if (m_currentOffer == &m_currentSelection->selection_offer)
                m_currentOffer = 0;
            m_currentSelection = 0;
        }
//        wl_display_remove_global(Compositor::instance()->wl_display(),
//                                 &m_retainedSelection->selection_offer.object);
        delete m_retainedSelection;
        m_retainedSelection = 0;
    }
    m_offerList.clear();
    struct wl_selection *selection = new struct wl_selection;
    memset(selection, 0, sizeof *selection);
    selection->resource.object.id = id;
    selection->resource.object.interface = &wl_selection_interface;
    selection->resource.object.implementation = (void (**)()) &selectionInterface;
    selection->resource.destroy = destroySelection;
    selection->client = client;
    selection->input_device = Compositor::instance()->inputDevice()->base();
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
        foreach (const QString &mimeType, m_offerList) {
            QByteArray mimeTypeBa = mimeType.toLatin1();
            wl_resource_post_event(&offer->resource,
                                 WL_SELECTION_OFFER_OFFER, mimeTypeBa.constData());
        }
        wl_resource_post_event(&offer->resource,
                             WL_SELECTION_OFFER_KEYBOARD_FOCUS, selection->input_device);
    }
}

void Selection::clearSelection()
{
    wl_display *dpy = Compositor::instance()->wl_display();
    QList<struct wl_client *> clients = Compositor::instance()->clients();

    if (m_currentSelection && clients.contains(m_currentSelection->client))
        wl_client_post_event(m_currentSelection->client,
                             &m_currentSelection->resource.object,
                             WL_SELECTION_CANCELLED);

    if (m_currentOffer)
        foreach (struct wl_client *client, clients)
            wl_client_post_event(client, &m_currentOffer->object,
                                 WL_SELECTION_OFFER_KEYBOARD_FOCUS, 0);

    m_currentSelection = 0;
    m_currentOffer = 0;
    m_offerList.clear();

    if (m_retainedSelection) {
        wl_display_remove_global(dpy, &m_retainedSelection->selection_offer.object);
        delete m_retainedSelection;
        m_retainedSelection = 0;
    }
}

void Selection::overrideSelection(QMimeData *data)
{
    clearSelection();

    m_retainedSelection = new wl_selection;
    memset(m_retainedSelection, 0, sizeof *m_retainedSelection);
    m_retainedSelection->client = 0; // indicates there is a special selection active
    m_retainedSelection->input_device = Compositor::instance()->inputDevice(); // needed for keyboard_focus

    m_offerList.append(data->formats());
    m_retainedData.clear();
    foreach (const QString &format, m_offerList)
        m_retainedData.setData(format, data->data(format));

    selActivate(0, m_retainedSelection, 0, 0);
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
