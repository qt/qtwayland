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

#ifndef WLINPUTDEVICE_H
#define WLINPUTDEVICE_H

#include "waylandobject.h"

#include <stdint.h>

#include <QtCore/QList>
#include <QtCore/QPoint>

class QTouchEvent;
class WaylandInputDevice;

namespace Wayland {

class Compositor;
class DataDevice;
class Surface;
class DataDeviceManager;

class InputDevice : public Object<struct wl_input_device>
{
public:
    InputDevice(WaylandInputDevice *handle, Compositor *compositor);
    ~InputDevice();

    void sendMousePressEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos = QPoint());
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &localPos, const QPoint &globalPos = QPoint());
    void sendMouseMoveEvent(const QPoint &localPos, const QPoint &globalPos = QPoint());
    void sendMouseMoveEvent(Surface *surface, const QPoint &localPos, const QPoint &globalPos = QPoint());

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendTouchPointEvent(int id, int x, int y, Qt::TouchPointState state);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendFullTouchEvent(QTouchEvent *event);

    Surface *keyboardFocus() const;
    void setKeyboardFocus(Surface *surface);

    Surface *mouseFocus() const;
    void setMouseFocus(Surface *surface, const QPoint &localPos, const QPoint &globalPos);

    void clientRequestedDataDevice(DataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    DataDevice *dataDevice(struct wl_client *client) const;
    void sendSelectionFocus(Surface *surface);

    Compositor *compositor() const;
    WaylandInputDevice *handle() const;

private:
    void cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev);

    WaylandInputDevice *m_handle;
    Compositor *m_compositor;
    QList<DataDevice *>m_data_devices;

    uint32_t toWaylandButton(Qt::MouseButton button);

    static void bind_func(struct wl_client *client, void *data,
                                uint32_t version, uint32_t id);
    static void input_device_attach(struct wl_client *client,
                             struct wl_resource *device_base,
                             uint32_t time,
                             struct wl_resource *buffer, int32_t x, int32_t y);
    const static struct wl_input_device_interface input_device_interface;
    static void destroy_resource(struct wl_resource *resource);
};

}

#endif // WLINPUTDEVICE_H
