/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QTWAYLAND_QWLTOUCH_P_H
#define QTWAYLAND_QWLTOUCH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/QWaylandDestroyListener>
#include <QtWaylandCompositor/QWaylandTouch>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QtCore/QPoint>
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandTouchPrivate : public QObjectPrivate, public QtWaylandServer::wl_touch
{
    Q_DECLARE_PUBLIC(QWaylandTouch)
public:
    explicit QWaylandTouchPrivate(QWaylandTouch *touch, QWaylandSeat *seat);

    QWaylandCompositor *compositor() const { return seat->compositor(); }

    uint sendDown(QWaylandSurface *surface, uint32_t time, int touch_id, const QPointF &position);
    void sendMotion(QWaylandClient *client, uint32_t time, int touch_id, const QPointF &position);
    uint sendUp(QWaylandClient *client, uint32_t time, int touch_id);

private:
    void touch_release(Resource *resource) override;
    int toSequentialWaylandId(int touchId);

    QWaylandSeat *seat = nullptr;
    QVarLengthArray<int, 10> ids;
};

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLTOUCH_P_H
