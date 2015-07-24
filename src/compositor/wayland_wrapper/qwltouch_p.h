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

#ifndef QTWAYLAND_QWLTOUCH_P_H
#define QTWAYLAND_QWLTOUCH_P_H

#include <QtCompositor/qwaylandexport.h>

#include <QtCore/QPoint>
#include <QtCore/QObject>

#include <QtCompositor/private/qwayland-server-wayland.h>

#include "qwllistener_p.h"

QT_BEGIN_NAMESPACE

class QWaylandSurfaceView;

namespace QtWayland {

class Compositor;
class Surface;
class Touch;

class Q_COMPOSITOR_EXPORT TouchGrabber {
public:
    TouchGrabber();
    virtual ~TouchGrabber();

    virtual void down(uint32_t time, int touch_id, const QPointF &position) = 0;
    virtual void up(uint32_t time, int touch_id) = 0;
    virtual void motion(uint32_t time, int touch_id, const QPointF &position) = 0;

    const Touch *touch() const;
    Touch *touch();
    void setTouch(Touch *touch);

private:
    Touch *m_touch;
};

class Q_COMPOSITOR_EXPORT Touch : public QObject, public QtWaylandServer::wl_touch, public TouchGrabber
{
public:
    explicit Touch(Compositor *compositor);

    void setFocus(QWaylandSurfaceView *surface);

    void startGrab(TouchGrabber *grab);
    void endGrab();

    void sendCancel();
    void sendFrame();

    void sendDown(int touch_id, const QPointF &position);
    void sendMotion(int touch_id, const QPointF &position);
    void sendUp(int touch_id);

    void down(uint32_t time, int touch_id, const QPointF &position);
    void up(uint32_t time, int touch_id);
    void motion(uint32_t time, int touch_id, const QPointF &position);

private:
    void focusDestroyed(void *data);
    void touch_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void touch_release(Resource *resource) Q_DECL_OVERRIDE;

    Compositor *m_compositor;

    QWaylandSurfaceView *m_focus;
    Resource *m_focusResource;
    WlListener m_focusDestroyListener;

    TouchGrabber *m_grab;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLTOUCH_P_H
