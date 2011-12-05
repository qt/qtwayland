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

#include "wldisplay.h"
#include "wlcompositor.h"

#include <QtCore/QDebug>

#include <sys/mman.h>

namespace Wayland {

ShmBuffer::ShmBuffer(struct wl_buffer *buffer)
    : m_buffer(buffer)
{
    m_buffer->user_data = this;
    m_data = wl_shm_buffer_get_data(m_buffer);
    m_stride = wl_shm_buffer_get_stride(m_buffer);

    damage();
}

ShmBuffer::~ShmBuffer()
{
}

QImage ShmBuffer::image() const
{
    return m_image;
}

QSize ShmBuffer::size() const
{
    return QSize(m_buffer->width, m_buffer->height);
}

void ShmBuffer::damage()
{
    QImage::Format imageFormat = QImage::Format_Invalid;

    imageFormat = QImage::Format_ARGB32_Premultiplied;

    m_image = QImage(static_cast<uchar *>(m_data),m_buffer->width, m_buffer->height,m_stride,imageFormat);

}

static ShmHandler *handlerInstance;

ShmHandler::ShmHandler(Display *display)
    : m_display(display)
{
    handlerInstance = this;
    m_shm = wl_shm_init(m_display->handle(),&shm_callbacks);
}

ShmHandler::~ShmHandler()
{
    wl_shm_finish(m_shm);
}

struct wl_shm_callbacks ShmHandler::shm_callbacks = {
    buffer_created_callback,
    buffer_damaged_callback,
    buffer_destroyed_callback
};

void ShmHandler::buffer_created_callback(struct wl_buffer *buffer)
{
    ShmBuffer *newBuffer = new ShmBuffer(buffer);
    Q_UNUSED(newBuffer);
}

void ShmHandler::buffer_damaged_callback(struct wl_buffer *buffer,
                      int32_t x, int32_t y,
                      int32_t width, int32_t height)
{
    Q_UNUSED(buffer);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    //damage has the responsibillity to update the QImage
    //for now we just recrate the entire QImage as we need a new
    //hash key for texture uploads
    static_cast<ShmBuffer *>(buffer->user_data)->damage();
}

void ShmHandler::buffer_destroyed_callback(struct wl_buffer *buffer)
{
    ShmBuffer *shmbuf = static_cast<ShmBuffer *>(buffer->user_data);
    for (int i = 0; i < handlerInstance->m_extraCallbacks.count(); ++i)
        handlerInstance->m_extraCallbacks.at(i)(shmbuf);
    delete shmbuf;
}

void ShmHandler::addDestroyCallback(DestroyCallback callback)
{
    if (!m_extraCallbacks.contains(callback))
        m_extraCallbacks.append(callback);
}

}
