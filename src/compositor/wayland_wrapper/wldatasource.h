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

#ifndef WLDATASOURCE_H
#define WLDATASOURCE_H

#include <wayland-server-protocol.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>

namespace Wayland {

class DataOffer;

class DataSource
{
public:
    DataSource(struct wl_client *client, uint32_t id, uint32_t time);
    ~DataSource();
    uint32_t time() const;
    QList<QByteArray> offerList() const;

    DataOffer *dataOffer() const;

    void postSendEvent(const QByteArray mimeType,int fd);
    struct wl_client *client() const;

private:
    uint32_t m_time;
    QList<QByteArray> m_offers;
    struct wl_resource *m_data_source_resource;

    DataOffer *m_data_offer;

    static struct wl_data_source_interface data_source_interface;
    static void offer(struct wl_client *client,
                  struct wl_resource *resource,
                  const char *type);
    static void destroy(struct wl_client *client,
                    struct wl_resource *resource);
    static void resource_destroy(struct wl_resource *resource);

};

}

#endif // WLDATASOURCE_H
