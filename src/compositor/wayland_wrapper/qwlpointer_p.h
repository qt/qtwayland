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

#ifndef QTWAYLAND_QWLPOINTER_P_H
#define QTWAYLAND_QWLPOINTER_P_H

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QObject>

#include <QtCompositor/private/qwayland-server-wayland.h>
#include <QtCompositor/QWaylandSurfaceView>
#include <QtCompositor/QWaylandSurface>

#include <stdint.h>

#include "qwllistener_p.h"

QT_BEGIN_NAMESPACE

class QWaylandSurfaceView;

namespace QtWayland {

class Compositor;
class InputDevice;
class Pointer;
class Surface;

class Q_COMPOSITOR_EXPORT PointerGrabber {
public:
    virtual ~PointerGrabber();

    virtual void focus() = 0;
    virtual void motion(uint32_t time) = 0;
    virtual void button(uint32_t time, Qt::MouseButton button, uint32_t state) = 0;

    Pointer *m_pointer;
};

class CurrentPosition
{
public:
    CurrentPosition()
        : m_view(Q_NULLPTR)
     {}

    void updatePosition(const QPointF &position)
    {
        Q_ASSERT(m_view || position.isNull());
        m_point = position;
        //we adjust if the mouse position is on the edge
        //to work around Qt's event propogation
        if (position.isNull())
            return;
        if (m_view->surface()) {
            QSizeF size(m_view->surface()->size());
            if (m_point.x() ==  size.width())
                m_point.rx() -= 0.01;

            if (m_point.y() == size.height())
                m_point.ry() -= 0.01;
        }
    }

    QPointF position() const { return m_point; }
    qreal x() const { return m_point.x(); }
    qreal y() const { return m_point.y(); }

    void setView(QWaylandSurfaceView *view) { m_view = view; }
    QWaylandSurfaceView *view() const { return m_view; }

    void setCurrent(QWaylandSurfaceView *view, const QPointF &position)
    {
        QPointF toSet = view || position.isNull() ? position : QPointF();
        setView(view);
        updatePosition(toSet);
    }

private:
    QWaylandSurfaceView *m_view;
    QPointF m_point;
};

class Q_COMPOSITOR_EXPORT Pointer : public QObject, public QtWaylandServer::wl_pointer, public PointerGrabber
{
public:
    Pointer(Compositor *compositor, InputDevice *seat);

    void setFocus(QWaylandSurfaceView *surface, const QPointF &position);

    void startGrab(PointerGrabber *currentGrab);
    void endGrab();
    PointerGrabber *currentGrab() const;
    Qt::MouseButton grabButton() const;
    uint32_t grabTime() const;
    uint32_t grabSerial() const;

    void setCurrent(QWaylandSurfaceView *surface, const QPointF &point);
    void setMouseFocus(QWaylandSurfaceView *surface, const QPointF &localPos, const QPointF &globalPos);

    void sendButton(uint32_t time, Qt::MouseButton button, uint32_t state);

    void sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos);
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos);
    void sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos);
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    QWaylandSurfaceView *focusSurface() const;
    QWaylandSurfaceView *current() const;
    QPointF position() const;
    QPointF currentPosition() const;
    Resource *focusResource() const;

    bool buttonPressed() const;

    void focus() Q_DECL_OVERRIDE;
    void motion(uint32_t time) Q_DECL_OVERRIDE;
    void button(uint32_t time, Qt::MouseButton button, uint32_t state) Q_DECL_OVERRIDE;

protected:
    void pointer_set_cursor(Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y) Q_DECL_OVERRIDE;
    void pointer_release(Resource *resource) Q_DECL_OVERRIDE;
    void pointer_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

private:
    void focusDestroyed(void *data);

    Compositor *m_compositor;
    InputDevice *m_seat;

    PointerGrabber *m_grab;
    Qt::MouseButton m_grabButton;
    uint32_t m_grabTime;
    uint32_t m_grabSerial;

    QPointF m_position;

    QWaylandSurfaceView *m_focus;
    Resource *m_focusResource;

    CurrentPosition m_currentPosition;

    int m_buttonCount;

    WlListener m_focusDestroyListener;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLPOINTER_P_H
