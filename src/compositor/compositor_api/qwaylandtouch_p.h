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

#include <QtWaylandCompositor/qwaylandexport.h>
#include <QtWaylandCompositor/QWaylandDestroyListener>
#include <QtWaylandCompositor/QWaylandTouch>
#include <QtWaylandCompositor/QWaylandInputDevice>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtCore/QPoint>
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class Q_COMPOSITOR_EXPORT QWaylandTouchPrivate : public QObjectPrivate, public QtWaylandServer::wl_touch
{
    Q_DECLARE_PUBLIC(QWaylandTouch)
public:
    explicit QWaylandTouchPrivate(QWaylandTouch *touch, QWaylandInputDevice *seat);

    QWaylandCompositor *compositor() const { return seat->compositor(); }

    void sendDown(int touch_id, const QPointF &position) { grab->down(compositor()->currentTimeMsecs(), touch_id, position);}
    void sendMotion(int touch_id, const QPointF &position) { grab->motion(compositor()->currentTimeMsecs(), touch_id, position); }
    void sendUp(int touch_id) { grab->up(compositor()->currentTimeMsecs(), touch_id); }

    void setFocusResource()
    {
        if (focusResource)
            return;

        QWaylandView *mouseFocus = seat->mouseFocus();
        if (!mouseFocus || !mouseFocus->surface())
            return;

        focusResource = resourceMap().value(mouseFocus->surface()->waylandClient());
    }
private:
    void resetFocusState();
    void touch_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void touch_release(Resource *resource) Q_DECL_OVERRIDE;

    QWaylandInputDevice *seat;

    Resource *focusResource;
    QWaylandDestroyListener focusDestroyListener;

    QWaylandDefaultTouchGrabber defaultGrab;
    QWaylandTouchGrabber *grab;
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLTOUCH_P_H
