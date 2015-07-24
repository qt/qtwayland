/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef WLDATADEVICE_H
#define WLDATADEVICE_H

#include <QtCompositor/private/qwayland-server-wayland.h>
#include <qwlpointer_p.h>

QT_BEGIN_NAMESPACE

class QWaylandSurfaceView;

namespace QtWayland {

class Compositor;
class DataSource;
class InputDevice;
class Surface;

class DataDevice : public QtWaylandServer::wl_data_device, public PointerGrabber
{
public:
    DataDevice(InputDevice *inputDevice);

    void setFocus(QtWaylandServer::wl_keyboard::Resource *focusResource);

    void setDragFocus(QWaylandSurfaceView *focus, const QPointF &localPosition);

    QWaylandSurfaceView *dragIcon() const;

    void sourceDestroyed(DataSource *source);

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;
protected:
    void data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial) Q_DECL_OVERRIDE;
    void data_device_set_selection(Resource *resource, struct ::wl_resource *source, uint32_t serial) Q_DECL_OVERRIDE;

private:
    Compositor *m_compositor;
    InputDevice *m_inputDevice;

    DataSource *m_selectionSource;

    struct ::wl_client *m_dragClient;
    DataSource *m_dragDataSource;

    QWaylandSurfaceView *m_dragFocus;
    Resource *m_dragFocusResource;

    QWaylandSurfaceView *m_dragIcon;
};

}

QT_END_NAMESPACE

#endif // WLDATADEVICE_H
