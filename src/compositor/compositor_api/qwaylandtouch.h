// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDTOUCH_H
#define QWAYLANDTOUCH_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

#include <QtCore/QObject>
#include <QtGui/QTouchEvent>

struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandTouch;
class QWaylandTouchPrivate;
class QWaylandSeat;
class QWaylandView;
class QWaylandClient;
class QWaylandSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTouch : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandTouch)
public:
    QWaylandTouch(QWaylandSeat *seat, QObject *parent = nullptr);

    QWaylandSeat *seat() const;
    QWaylandCompositor *compositor() const;

    virtual uint sendTouchPointEvent(QWaylandSurface *surface, int id, const QPointF &position, Qt::TouchPointState state);
    virtual void sendFrameEvent(QWaylandClient *client);
    virtual void sendCancelEvent(QWaylandClient *client);
    virtual void sendFullTouchEvent(QWaylandSurface *surface, QTouchEvent *event);

    virtual void addClient(QWaylandClient *client, uint32_t id, uint32_t version);
};

QT_END_NAMESPACE

#endif  /*QWAYLANDTOUCH_H*/
