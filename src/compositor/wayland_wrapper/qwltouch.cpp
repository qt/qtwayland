/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwltouch_p.h"

#include "qwlcompositor_p.h"
#include "qwlsurface_p.h"
#include "qwaylandsurfaceview.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

Touch::Touch(Compositor *compositor)
    : wl_touch()
    , m_compositor(compositor)
    , m_focus()
    , m_focusResource()
    , m_grab(this)
{
    m_grab->setTouch(this);
    connect(&m_focusDestroyListener, &WlListener::fired, this, &Touch::focusDestroyed);
}

void Touch::setFocus(QWaylandSurfaceView *surface)
{
    m_focusDestroyListener.reset();
    if (surface)
        m_focusDestroyListener.listenForDestruction(surface->surface()->handle()->resource()->handle);

    m_focus = surface;
    m_focusResource = surface ? resourceMap().value(surface->surface()->handle()->resource()->client()) : 0;
}

void Touch::startGrab(TouchGrabber *grab)
{
    m_grab = grab;
    grab->setTouch(this);
}

void Touch::endGrab()
{
    m_grab = this;
}

void Touch::focusDestroyed(void *data)
{
    Q_UNUSED(data)
    m_focusDestroyListener.reset();

    m_focus = 0;
    m_focusResource = 0;
}

void Touch::touch_destroy_resource(Resource *resource)
{
    if (m_focusResource == resource)
        m_focusResource = 0;
}

void Touch::touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void Touch::sendCancel()
{
    if (m_focusResource)
        send_cancel(m_focusResource->handle);
}

void Touch::sendFrame()
{
    if (m_focusResource)
        send_frame(m_focusResource->handle);
}

void Touch::sendDown(int touch_id, const QPointF &position)
{
    m_grab->down(m_compositor->currentTimeMsecs(), touch_id, position);
}

void Touch::sendMotion(int touch_id, const QPointF &position)
{
    m_grab->motion(m_compositor->currentTimeMsecs(), touch_id, position);
}

void Touch::sendUp(int touch_id)
{
    m_grab->up(m_compositor->currentTimeMsecs(), touch_id);
}

void Touch::down(uint32_t time, int touch_id, const QPointF &position)
{
    if (!m_focusResource || !m_focus)
        return;

    uint32_t serial = wl_display_next_serial(m_compositor->wl_display());

    send_down(m_focusResource->handle, serial, time, m_focus->surface()->handle()->resource()->handle, touch_id,
              wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

void Touch::up(uint32_t time, int touch_id)
{
    if (!m_focusResource)
        return;

    uint32_t serial = wl_display_next_serial(m_compositor->wl_display());

    send_up(m_focusResource->handle, serial, time, touch_id);
}

void Touch::motion(uint32_t time, int touch_id, const QPointF &position)
{
    if (!m_focusResource)
        return;

    send_motion(m_focusResource->handle, time, touch_id,
                wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

TouchGrabber::TouchGrabber()
    : m_touch(0)
{
}

TouchGrabber::~TouchGrabber()
{
}

const Touch *TouchGrabber::touch() const
{
    return m_touch;
}

Touch *TouchGrabber::touch()
{
    return m_touch;
}

void TouchGrabber::setTouch(Touch *touch)
{
    m_touch = touch;
}

} // namespace QtWayland

QT_END_NAMESPACE
