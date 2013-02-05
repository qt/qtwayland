/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwldatadevicemanager_p.h"

#include "qwldatadevice_p.h"
#include "qwldatasource_p.h"
#include "qwlinputdevice_p.h"
#include "qwlcompositor_p.h"
#include "qwldataoffer_p.h"
#include "qwlsurface_p.h"
#include "qwaylandmimehelper.h"

#include <QtCore/QDebug>
#include <QtCore/QSocketNotifier>
#include <fcntl.h>
#include <QtCore/private/qcore_unix_p.h>
#include <QtCore/QFile>

QT_BEGIN_NAMESPACE

namespace QtWayland {

DataDeviceManager::DataDeviceManager(Compositor *compositor)
    : m_compositor(compositor)
    , m_current_selection_source(0)
    , m_retainedReadNotifier(0)
    , m_compositorOwnsSelection(false)
{
    wl_display_add_global(compositor->wl_display(), &wl_data_device_manager_interface, this, DataDeviceManager::bind_func_drag);
}

void DataDeviceManager::setCurrentSelectionSource(DataSource *source)
{
    if (m_current_selection_source
            && m_current_selection_source->time() > source->time()) {
        qDebug() << "Trying to set older selection";
        return;
    }

    m_compositorOwnsSelection = false;

    finishReadFromClient();

    m_current_selection_source = source;
    source->setManager(this);

    // When retained selection is enabled, the compositor will query all the data from the client.
    // This makes it possible to
    //    1. supply the selection after the offering client is gone
    //    2. make it possible for the compositor to participate in copy-paste
    // The downside is decreased performance, therefore this mode has to be enabled
    // explicitly in the compositors.
    if (m_compositor->wantsRetainedSelection()) {
        m_retainedData.clear();
        m_retainedReadIndex = 0;
        retain();
    }
}

void DataDeviceManager::sourceDestroyed(DataSource *source)
{
    Q_UNUSED(source);
    if (m_current_selection_source == source)
        finishReadFromClient();
}

void DataDeviceManager::retain()
{
    QList<QByteArray> offers = m_current_selection_source->offerList();
    finishReadFromClient();
    if (m_retainedReadIndex >= offers.count()) {
        m_compositor->feedRetainedSelectionData(&m_retainedData);
        return;
    }
    QByteArray mimeType = offers.at(m_retainedReadIndex);
    m_retainedReadBuf.clear();
    int fd[2];
    if (pipe(fd) == -1) {
        qWarning("Clipboard: Failed to create pipe");
        return;
    }
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL, 0) | O_NONBLOCK);
    m_current_selection_source->postSendEvent(mimeType, fd[1]);
    close(fd[1]);
    m_retainedReadNotifier = new QSocketNotifier(fd[0], QSocketNotifier::Read, this);
    connect(m_retainedReadNotifier, SIGNAL(activated(int)), SLOT(readFromClient(int)));
}

void DataDeviceManager::finishReadFromClient(bool exhausted)
{
    Q_UNUSED(exhausted);
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

void DataDeviceManager::readFromClient(int fd)
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
            QList<QByteArray> offers = m_current_selection_source->offerList();
            QString mimeType = QString::fromLatin1(offers.at(m_retainedReadIndex));
            m_retainedData.setData(mimeType, m_retainedReadBuf);
            ++m_retainedReadIndex;
            retain();
        }
    } else {
        m_retainedReadBuf.append(buf, n);
    }
}

DataSource *DataDeviceManager::currentSelectionSource()
{
    return m_current_selection_source;
}

struct wl_display *DataDeviceManager::display() const
{
    return m_compositor->wl_display();
}

void DataDeviceManager::bind_func_drag(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_client_add_object(client,&wl_data_device_manager_interface,&drag_interface,id,data);
}

void DataDeviceManager::bind_func_data(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(data);
    Q_UNUSED(version);
    Q_UNUSED(id);
}

void DataDeviceManager::get_data_device(struct wl_client *client,
                      struct wl_resource *data_device_manager_resource,
                      uint32_t id,
                      struct wl_resource *input_device_resource)
{
    DataDeviceManager *data_device_manager = static_cast<DataDeviceManager *>(data_device_manager_resource->data);
    InputDevice *input_device = resolve<InputDevice>(input_device_resource);
    input_device->clientRequestedDataDevice(data_device_manager,client,id);
}

void DataDeviceManager::create_data_source(struct wl_client *client,
                               struct wl_resource *data_device_manager_resource,
                               uint32_t id)
{
    Q_UNUSED(data_device_manager_resource);
    new DataSource(client,id, Compositor::currentTimeMsecs());
}

struct wl_data_device_manager_interface DataDeviceManager::drag_interface = {
    DataDeviceManager::create_data_source,
    DataDeviceManager::get_data_device
};

void DataDeviceManager::overrideSelection(const QMimeData &mimeData)
{
    QStringList formats = mimeData.formats();
    if (formats.isEmpty())
        return;

    m_retainedData.clear();
    foreach (const QString &format, formats)
        m_retainedData.setData(format, mimeData.data(format));

    m_compositor->feedRetainedSelectionData(&m_retainedData);

    m_compositorOwnsSelection = true;

    InputDevice *dev = m_compositor->defaultInputDevice();
    Surface *focusSurface = dev->keyboardFocus();
    if (focusSurface)
        offerFromCompositorToClient(
                    dev->dataDevice(focusSurface->base()->resource.client)->dataDeviceResource());
}

bool DataDeviceManager::offerFromCompositorToClient(wl_resource *clientDataDeviceResource)
{
    if (!m_compositorOwnsSelection)
        return false;

    wl_client *client = clientDataDeviceResource->client;
    //qDebug("compositor offers %d types to %p", m_retainedData.formats().count(), client);

    struct wl_resource *selectionOffer =
             wl_client_new_object(client, &wl_data_offer_interface, &compositor_offer_interface, this);
    wl_data_device_send_data_offer(clientDataDeviceResource, selectionOffer);
    foreach (const QString &format, m_retainedData.formats()) {
        QByteArray ba = format.toLatin1();
        wl_data_offer_send_offer(selectionOffer, ba.constData());
    }
    wl_data_device_send_selection(clientDataDeviceResource, selectionOffer);

    return true;
}

void DataDeviceManager::offerRetainedSelection(wl_resource *clientDataDeviceResource)
{
    if (m_retainedData.formats().isEmpty())
        return;

    m_compositorOwnsSelection = true;
    offerFromCompositorToClient(clientDataDeviceResource);
}

void DataDeviceManager::comp_accept(wl_client *, wl_resource *, uint32_t, const char *)
{
}

void DataDeviceManager::comp_receive(wl_client *client, wl_resource *resource, const char *mime_type, int32_t fd)
{
    Q_UNUSED(client);
    DataDeviceManager *self = static_cast<DataDeviceManager *>(resource->data);
    //qDebug("client %p wants data for type %s from compositor", client, mime_type);
    QByteArray content = QWaylandMimeHelper::getByteArray(&self->m_retainedData, QString::fromLatin1(mime_type));
    if (!content.isEmpty()) {
        QFile f;
        if (f.open(fd, QIODevice::WriteOnly))
            f.write(content);
    }
    close(fd);
}

void DataDeviceManager::comp_destroy(wl_client *, wl_resource *)
{
}

const struct wl_data_offer_interface DataDeviceManager::compositor_offer_interface = {
    DataDeviceManager::comp_accept,
    DataDeviceManager::comp_receive,
    DataDeviceManager::comp_destroy
};

} //namespace

QT_END_NAMESPACE
