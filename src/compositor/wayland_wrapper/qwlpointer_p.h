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
#include <QtCompositor/QWaylandDestroyListener>
#include <QtCompositor/QWaylandPointer>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtCore/private/qobject_p.h>

#include <QtCompositor/private/qwayland-server-wayland.h>
#include <QtCompositor/QWaylandSurfaceView>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/QWaylandInputDevice>

#include <stdint.h>

QT_BEGIN_NAMESPACE

class QWaylandSurfaceView;

namespace QtWayland {

class Compositor;
class Surface;

class CurrentPosition
{
public:
    CurrentPosition()
        : m_view(Q_NULLPTR)
     {}

    void reset()
    {
        m_point = QPointF();
        m_view = Q_NULLPTR;
    }

    void updatePosition(QWaylandSurfaceView *view, const QPointF &position)
    {
        m_view = view;
        m_point = position;
        //we adjust if the mouse position is on the edge
        //to work around Qt's event propogation
        if (!m_view || position.isNull())
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

    QWaylandSurfaceView *view() const { return m_view; }

private:
    QWaylandSurfaceView *m_view;
    QPointF m_point;
};


} // namespace QtWayland

class Q_COMPOSITOR_EXPORT QWaylandPointerPrivate : public QObjectPrivate
                                                 , public QtWaylandServer::wl_pointer
{
    Q_DECLARE_PUBLIC(QWaylandPointer)
public:
    QWaylandPointerPrivate(QWaylandPointer *pointer, QWaylandInputDevice *seat);

    QWaylandOutput *output() const { return m_output; }
    void setOutput(QWaylandOutput *output)
    {
        if (m_output == output) return;
        Q_Q(QWaylandPointer);
        m_output = output;
        q->outputChanged();
    }


    void startGrab(QWaylandPointerGrabber *currentGrab);
    void endGrab();
    QWaylandPointerGrabber *currentGrab() const;
    Qt::MouseButton grabButton() const;
    uint32_t grabTime() const;
    uint32_t grabSerial() const;

    void sendMousePressEvent(Qt::MouseButton button);
    void sendMouseReleaseEvent(Qt::MouseButton button);
    void sendMouseMoveEvent(QWaylandSurfaceView *view, const QPointF &localPos, const QPointF &outputSpacePos);
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    Resource *focusResource() const { return m_focusResource; }
    QWaylandSurfaceView *focusView() const { return m_currentPosition.view(); }

    bool buttonPressed() const;

    QWaylandInputDevice *seat() const { return m_seat; }
    QWaylandCompositor *compositor() const { return m_seat->compositor(); }

    void resetCurrentState() { m_currentPosition.reset(); }
    QWaylandSurfaceView *currentView() const { return m_currentPosition.view(); }
    QPointF currentSpacePosition() const { return m_spacePosition; }
    QPointF currentLocalPosition() const { return m_currentPosition.position(); }
protected:
    void pointer_set_cursor(Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y) Q_DECL_OVERRIDE;
    void pointer_release(Resource *resource) Q_DECL_OVERRIDE;
    void pointer_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

private:
    void focusDestroyed(void *data);

    QWaylandInputDevice *m_seat;
    QWaylandOutput *m_output;
    QWaylandDefaultPointerGrabber m_defaultGrab;
    QtWayland::CurrentPosition m_currentPosition;
    QPointF m_spacePosition;

    QWaylandPointerGrabber *m_grab;
    Qt::MouseButton m_grabButton;
    uint32_t m_grabTime;
    uint32_t m_grabSerial;

    Resource *m_focusResource;


    int m_buttonCount;

    QWaylandDestroyListener m_focusDestroyListener;
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLPOINTER_P_H
