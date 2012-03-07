/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockcompositor.h"
#include "mockshm.h"

namespace Impl {

ShmBuffer::ShmBuffer(wl_buffer *buffer)
    : m_buffer(buffer)
{
    refresh();
}

void ShmBuffer::refresh()
{
    m_image = QImage(static_cast<uint8_t *>(wl_shm_buffer_get_data(m_buffer)),
                     m_buffer->width, m_buffer->height,
                     wl_shm_buffer_get_stride(m_buffer),
                     QImage::Format_ARGB32_Premultiplied);
}

QImage ShmBuffer::image() const
{
    return m_image;
}

static void shm_buffer_created(wl_buffer *buffer)
{
    buffer->user_data = new ShmBuffer(buffer);
}

static void shm_buffer_damaged(wl_buffer *buffer,
                               int32_t x, int32_t y,
                               int32_t width, int32_t height)
{
    Q_UNUSED(QRect(x, y, width, height));
    static_cast<ShmBuffer *>(buffer->user_data)->refresh();
}

static void shm_buffer_destroyed(wl_buffer *buffer)
{
    delete static_cast<ShmBuffer *>(buffer->user_data);
}

void Compositor::initShm()
{
    static struct wl_shm_callbacks shmCallbacks = {
        shm_buffer_created,
        shm_buffer_damaged,
        shm_buffer_destroyed
    };

    m_shm = wl_shm_init(m_display, &shmCallbacks);
}

}

