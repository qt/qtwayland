/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wltouch.h"
#include "wlsurface.h"
#include <QTouchEvent>
#include <QWindow>

namespace Wayland {

static void dummy(wl_client *, wl_resource *)
{
}

const struct wl_touch_interface TouchExtensionGlobal::touch_interface = {
    dummy
};

TouchExtensionGlobal::TouchExtensionGlobal(Compositor *compositor)
    : m_compositor(compositor)
{
    wl_display_add_global(compositor->wl_display(),
                          &wl_touch_interface,
                          this,
                          TouchExtensionGlobal::bind_func);
}

void TouchExtensionGlobal::destroy_resource(wl_resource *resource)
{
    TouchExtensionGlobal *self = static_cast<TouchExtensionGlobal *>(resource->data);
    self->m_resources.removeOne(resource);
    free(resource);
}

void TouchExtensionGlobal::bind_func(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_resource *resource = wl_client_add_object(client, &wl_touch_interface, &touch_interface, id, data);
    resource->destroy = destroy_resource;
    TouchExtensionGlobal *self = static_cast<TouchExtensionGlobal *>(resource->data);
    self->m_resources.append(resource);
}

static inline int toFixed(qreal f)
{
    return int(f * 10000);
}

void TouchExtensionGlobal::postTouchEvent(QTouchEvent *event, Surface *surface)
{
    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    const int pointCount = points.count();
    if (!pointCount)
        return;

    QPointF surfacePos = surface->pos();
    uint32_t time = m_compositor->currentTimeMsecs();
    const int rescount = m_resources.count();
    for (int res = 0; res < rescount; ++res) {
        wl_resource *target = m_resources.at(res);

        for (int i = 0; i < pointCount; ++i) {
            const QTouchEvent::TouchPoint &tp(points.at(i));
            uint32_t id = tp.id();
            uint32_t state = tp.state();
            uint32_t flags = tp.flags();
            QPointF p = tp.pos() - surfacePos; // surface-relative
            int x = toFixed(p.x());
            int y = toFixed(p.y());
            int nx = toFixed(tp.normalizedPos().x());
            int ny = toFixed(tp.normalizedPos().y());
            int w = toFixed(tp.rect().width());
            int h = toFixed(tp.rect().height());
            int vx = toFixed(tp.velocity().x());
            int vy = toFixed(tp.velocity().y());
            uint32_t pressure = uint32_t(tp.pressure() * 255);
            wl_array *rawData = 0;
            wl_resource_post_event(target, WL_TOUCH_TOUCH,
                                   time, id, state,
                                   x, y, nx, ny, w, h,
                                   pressure, vx, vy,
                                   flags, rawData);
        }

        wl_resource_post_event(target, WL_TOUCH_TOUCH_FRAME);
    }
}

}
