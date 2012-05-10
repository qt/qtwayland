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

#ifndef WAYLANDINPUT_H
#define WAYLANDINPUT_H

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>

#include "waylandexport.h"

class WaylandCompositor;
class WaylandSurface;
class QKeyEvent;
class QTouchEvent;

namespace Wayland {
class InputDevice;
}

class Q_COMPOSITOR_EXPORT WaylandInputDevice
{
public:
    WaylandInputDevice(WaylandCompositor *compositor);
    ~WaylandInputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseMoveEvent(WaylandSurface *surface , const QPointF &localPos, const QPointF &globalPos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendFullKeyEvent(QKeyEvent *event);

    void sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullTouchEvent(QTouchEvent *event);

    WaylandSurface *keyboardFocus() const;
    void setKeyboardFocus(WaylandSurface *surface);

    WaylandSurface *mouseFocus() const;
    void setMouseFocus(WaylandSurface *surface, const QPointF &local_pos, const QPointF &global_pos = QPointF());

    WaylandCompositor *compositor() const;
    Wayland::InputDevice *handle() const;

private:
    Wayland::InputDevice *d;
    Q_DISABLE_COPY(WaylandInputDevice)
};

#endif // WAYLANDINPUT_H
