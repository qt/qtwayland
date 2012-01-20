/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandinput.h"

#include "wlinputdevice.h"
#include "waylandcompositor.h"
#include "wlsurface.h"
#include "wlcompositor.h"

WaylandInputDevice::WaylandInputDevice(WaylandCompositor *compositor)
    : d(new Wayland::InputDevice(this,compositor->handle()))
{
}

void WaylandInputDevice::sendMousePressEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos)
{
    d->sendMousePressEvent(button,localPos,globalPos);
}

void WaylandInputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos)
{
    d->sendMouseReleaseEvent(button,localPos,globalPos);
}

void WaylandInputDevice::sendMouseMoveEvent(const QPoint &localPos, const QPoint &globalPos)
{
    d->sendMouseMoveEvent(localPos,globalPos);
}

/** Convenience function that will set the mouse focus to the surface, then send the mouse move event.
 *  If the mouse focus is the same surface as the surface passed in, then only the move event is sent
 **/
void WaylandInputDevice::sendMouseMoveEvent(WaylandSurface *surface, const QPoint &localPos, const QPoint &globalPos)
{
    Wayland::Surface *wlsurface = surface? surface->handle():0;
    d->sendMouseMoveEvent(wlsurface,localPos,globalPos);
}

void WaylandInputDevice::sendKeyPressEvent(uint code)
{
    d->sendKeyPressEvent(code);
}

void WaylandInputDevice::sendKeyReleaseEvent(uint code)
{
    d->sendKeyReleaseEvent(code);
}

void WaylandInputDevice::sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state)
{
    d->sendTouchPointEvent(id,x,y,state);
}

void WaylandInputDevice::sendTouchFrameEvent()
{
    d->sendTouchFrameEvent();
}

void WaylandInputDevice::sendTouchCancelEvent()
{
    d->sendTouchCancelEvent();
}

void WaylandInputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    d->sendFullTouchEvent(event);
}

WaylandSurface *WaylandInputDevice::keyboardFocus() const
{
    Wayland::Surface *wlsurface = d->keyboardFocus();
    if (wlsurface)
        return  wlsurface->handle();
    return 0;
}

void WaylandInputDevice::setKeyboardFocus(WaylandSurface *surface)
{
    Wayland::Surface *wlsurface = surface?surface->handle():0;
    d->setKeyboardFocus(wlsurface);
}

WaylandSurface *WaylandInputDevice::mouseFocus() const
{
    Wayland::Surface *wlsurface = d->mouseFocus();
    if (wlsurface)
        return  wlsurface->handle();
    return 0;
}

void WaylandInputDevice::setMouseFocus(WaylandSurface *surface, const QPoint &localPos, const QPoint &globalPos)
{
    Wayland::Surface *wlsurface = surface?surface->handle():0;
    d->setMouseFocus(wlsurface,localPos,globalPos);
}

WaylandCompositor *WaylandInputDevice::compositor() const
{
    return d->compositor()->qtCompositor();
}

Wayland::InputDevice *WaylandInputDevice::handle() const
{
    return d;
}


