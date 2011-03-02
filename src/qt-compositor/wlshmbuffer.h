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

#ifndef WL_SHMBUFFER_H
#define WL_SHMBUFFER_H

#include "wlbuffer.h"
#include "wlsurface.h"

#include <QtCore/QRect>
#include <QtGui/QImage>
namespace Wayland {

struct Surface;
class ShmBuffer : public Buffer
{
public:
    ShmBuffer(int fd,
              Compositor *compositor,
              const QSize &size,
              uint stride,
              struct wl_visual *visual);
    ~ShmBuffer();

    void attach(Surface *surface);
    void damage(Surface *surface, const QRect &rect);

    Buffer::BufferType type() const {return Buffer::Shm;}

    QImage image() const;
    QSize size() const;

private:
    int m_stride;
    void *m_data;
};

class ShmHandler : public Object<struct wl_object>
{
public:
    ShmHandler(Compositor *compositor);

    ShmBuffer *createBuffer(int fd, const QSize &size, uint32_t stride, struct wl_visual *visual);
private:
    Compositor *m_compositor;
};

void shm_create_buffer(struct wl_client *client,
                       struct wl_shm *shm,
                       uint32_t id,
                       int fd,
                       int width,
                       int height,
                       uint32_t stride,
                       struct wl_visual *visual);

const struct wl_shm_interface shm_interface = {
    shm_create_buffer
};

}

#endif //WL_SHMBUFFER_H
