/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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

#ifndef WL_SHMBUFFER_H
#define WL_SHMBUFFER_H

#include "waylandobject.h"

#include <QtCore/QRect>
#include <QtGui/QImage>


namespace Wayland {

class Surface;
class Display;

class ShmBuffer
{
public:
    ShmBuffer(struct wl_buffer *buffer);
    ~ShmBuffer();

    QImage image() const;
    QSize size() const;

    void damage();

private:
    struct wl_buffer *m_buffer;
    int m_stride;
    void *m_data;
    QImage m_image;
};

class ShmHandler
{
public:
    ShmHandler(Display *display);
    ~ShmHandler();

private:
    Display *m_display;
    struct wl_shm *m_shm;

    static struct wl_shm_callbacks shm_callbacks;
    static void buffer_created_callback(struct wl_buffer *buffer);
    static void buffer_damaged_callback(struct wl_buffer *buffer,
                          int32_t x, int32_t y,
                          int32_t width, int32_t height);
    static void buffer_destroyed_callback(struct wl_buffer *buffer);
};

}

#endif //WL_SHMBUFFER_H
