// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDPOINTER_P_H
#define QWAYLANDPOINTER_P_H

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
#include <QtWaylandCompositor/QWaylandPointer>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandSeat>

#include <QtCore/qpointer.h>

#include <stdint.h>

QT_BEGIN_NAMESPACE

class QWaylandView;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandPointerPrivate : public QObjectPrivate
                                                 , public QtWaylandServer::wl_pointer
{
    Q_DECLARE_PUBLIC(QWaylandPointer)
public:
    QWaylandPointerPrivate(QWaylandPointer *pointer, QWaylandSeat *seat);

    QWaylandCompositor *compositor() const { return seat->compositor(); }

protected:
    void pointer_set_cursor(Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y) override;
    void pointer_release(Resource *resource) override;

private:
    uint sendButton(Qt::MouseButton button, uint32_t state);
    void sendMotion();
    void sendEnter(QWaylandSurface *surface);
    void sendLeave();
    void ensureEntered(QWaylandSurface *surface);

    QWaylandSeat *seat = nullptr;
    QWaylandOutput *output = nullptr;
    QPointer<QWaylandSurface> enteredSurface;

    QPointF localPosition;
    QPointF spacePosition;

    uint enterSerial = 0;

    int buttonCount = 0;

    QWaylandDestroyListener enteredSurfaceDestroyListener;

    static QWaylandSurfaceRole s_role;
};

QT_END_NAMESPACE

#endif // QWAYLANDPOINTER_P_H
