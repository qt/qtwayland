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

#include "qwaylandinput.h"

#include "qwlinputdevice_p.h"
#include "qwlkeyboard_p.h"
#include "qwaylandcompositor.h"
#include "qwlsurface_p.h"
#include "qwlcompositor_p.h"

QT_BEGIN_NAMESPACE

QWaylandInputDevice::QWaylandInputDevice(QWaylandCompositor *compositor)
    : d(new QtWayland::InputDevice(this,compositor->handle()))
{
}

QWaylandInputDevice::~QWaylandInputDevice()
{
    delete d;
}

void QWaylandInputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMousePressEvent(button,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseReleaseEvent(button,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    d->sendMouseMoveEvent(localPos,globalPos);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void QWaylandInputDevice::sendMouseMoveEvent(QWaylandSurface *surface, const QPointF &localPos, const QPointF &globalPos)
{
    QtWayland::Surface *wlsurface = surface? surface->handle():0;
    d->sendMouseMoveEvent(wlsurface,localPos,globalPos);
}

void QWaylandInputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    d->sendMouseWheelEvent(orientation, delta);
}

void QWaylandInputDevice::sendKeyPressEvent(uint code)
{
    d->keyboardDevice()->sendKeyPressEvent(code);
}

void QWaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    d->keyboardDevice()->sendKeyReleaseEvent(code);
}

void QWaylandInputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    d->sendTouchPointEvent(id,x,y,state);
}

void QWaylandInputDevice::sendTouchFrameEvent()
{
    d->sendTouchFrameEvent();
}

void QWaylandInputDevice::sendTouchCancelEvent()
{
    d->sendTouchCancelEvent();
}

void QWaylandInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    d->sendFullTouchEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    d->sendFullKeyEvent(event);
}

void QWaylandInputDevice::sendFullKeyEvent(QWaylandSurface *surface, QKeyEvent *event)
{
    d->sendFullKeyEvent(surface->handle(), event);
}

QWaylandSurface *QWaylandInputDevice::keyboardFocus() const
{
    QtWayland::Surface *wlsurface = d->keyboardFocus();
    if (wlsurface)
        return  wlsurface->waylandSurface();
    return 0;
}

bool QWaylandInputDevice::setKeyboardFocus(QWaylandSurface *surface)
{
    QtWayland::Surface *wlsurface = surface?surface->handle():0;
    return d->setKeyboardFocus(wlsurface);
}

QWaylandSurface *QWaylandInputDevice::mouseFocus() const
{
    QtWayland::Surface *wlsurface = d->mouseFocus();
    if (wlsurface)
        return  wlsurface->waylandSurface();
    return 0;
}

void QWaylandInputDevice::setMouseFocus(QWaylandSurface *surface, const QPointF &localPos, const QPointF &globalPos)
{
    QtWayland::Surface *wlsurface = surface?surface->handle():0;
    d->setMouseFocus(wlsurface,localPos,globalPos);
}

QWaylandCompositor *QWaylandInputDevice::compositor() const
{
    return d->compositor()->waylandCompositor();
}

QtWayland::InputDevice *QWaylandInputDevice::handle() const
{
    return d;
}

QT_END_NAMESPACE
