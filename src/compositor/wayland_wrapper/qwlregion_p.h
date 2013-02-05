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

#ifndef WL_REGION_H
#define WL_REGION_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/qwaylandobject.h>

#include <QRegion>

#include <wayland-util.h>

QT_BEGIN_NAMESPACE

namespace QtWayland {

class Q_COMPOSITOR_EXPORT Region : public Object<wl_resource>
{
public:
    Region(struct wl_client *client, uint32_t id);
    ~Region();

    uint id() const { return base()->object.id; }

    QRegion region() const { return m_region; }

    static const struct wl_region_interface region_interface;

private:
    Q_DISABLE_COPY(Region)

    QRegion m_region;

    static void region_destroy(wl_client *client, wl_resource *region);
    static void region_add(wl_client *client, wl_resource *region,
                           int32_t x, int32_t y, int32_t w, int32_t h);
    static void region_subtract(wl_client *client, wl_resource *region,
                                int32_t x, int32_t y, int32_t w, int32_t h);
};

}

QT_END_NAMESPACE

#endif // WL_REGION_H

