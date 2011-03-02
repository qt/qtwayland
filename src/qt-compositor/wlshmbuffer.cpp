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

#include "wlshmbuffer.h"

#include <QtCore/QDebug>

#include <sys/mman.h>

namespace Wayland {

void shm_buffer_attach(struct wl_buffer *buffer, struct wl_surface *surface)
{
    wayland_cast<ShmBuffer *>(buffer)->attach(wayland_cast<Surface *>(surface));
}

void shm_buffer_damage(struct wl_buffer *buffer, struct wl_surface *surface, int x, int y, int width, int height)
{
    wayland_cast<ShmBuffer *>(buffer)->damage(wayland_cast<Surface *>(surface), QRect(x, y, width, height));
}


void shm_buffer_destroy(struct wl_resource *resource, struct wl_client *)
{
    delete wayland_cast<ShmBuffer *>((wl_buffer *)resource);
}

void buffer_interface_destroy(struct wl_client *client, struct wl_buffer *buffer)
{
    Q_UNUSED(client);
    Q_UNUSED(buffer);
    qDebug() << "buffer_interface_destroy()";
}

const struct wl_buffer_interface buffer_interface = {
    buffer_interface_destroy
};

void shm_create_buffer(struct wl_client *client,
                       struct wl_shm *shm,
                       uint32_t id,
                       int fd,
                       int width,
                       int height,
                       uint32_t stride,
                       struct wl_visual *visual)
{
    ShmHandler *shmHandler = reinterpret_cast<ShmHandler *>(shm);

    ShmBuffer *buffer = shmHandler->createBuffer(fd,QSize(width,height),stride,visual);

    addClientResource(client, &buffer->base()->resource, id, &wl_buffer_interface,
            &buffer_interface, shm_buffer_destroy);
}

ShmBuffer::ShmBuffer(int fd,
                     Compositor *compositor,
                     const QSize &size,
                     uint stride,
                     struct wl_visual *visual)
    : Buffer(compositor,visual,size)
{
    m_stride = stride;
    m_data = mmap(NULL, stride * size.height(), PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
}

ShmBuffer::~ShmBuffer()
{
    munmap(m_data, m_stride * base()->height);
}

void ShmBuffer::attach(Surface *surface)
{
    printf("ShmBuffer::attach(%p)\n", surface);
}

void ShmBuffer::damage(Surface *surface, const QRect &rect)
{
    printf("ShmBuffer::damage(%p, QRect(%d %d - %dx%d))\n", surface, rect.x(), rect.y(),
            rect.width(), rect.height());
}

QImage ShmBuffer::image() const
{
    if (m_data)
        return QImage(static_cast<const uchar *>(m_data), base()->width, base()->height, m_stride, QImage::Format_ARGB32_Premultiplied);
    return QImage();
}

QSize ShmBuffer::size() const
{
    return QSize(base()->width, base()->height);
}

ShmHandler::ShmHandler(Wayland::Compositor *compositor)
    :m_compositor(compositor)
{
}


ShmBuffer *ShmHandler::createBuffer(int fd, const QSize &size, uint32_t stride, struct wl_visual *visual)
{
    ShmBuffer *buffer = new ShmBuffer(fd,m_compositor,size,stride,visual);
    return buffer;
}

}
