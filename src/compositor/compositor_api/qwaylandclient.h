/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QWAYLANDCLIENT_H
#define QWAYLANDCLIENT_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

#include <QObject>

#include <signal.h>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandClientPrivate;
class QWaylandCompositor;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandClient : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandClient)

    Q_PROPERTY(QWaylandCompositor *compositor READ compositor CONSTANT)
    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
    Q_MOC_INCLUDE("qwaylandcompositor.h")

    QML_NAMED_ELEMENT(WaylandClient)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    ~QWaylandClient() override;

    static QWaylandClient *fromWlClient(QWaylandCompositor *compositor, wl_client *wlClient);

    QWaylandCompositor *compositor() const;

    wl_client *client() const;

    qint64 userId() const;
    qint64 groupId() const;

    qint64 processId() const;

    Q_INVOKABLE void kill(int signal = SIGTERM);

public Q_SLOTS:
    void close();

private:
    explicit QWaylandClient(QWaylandCompositor *compositor, wl_client *client);
};

QT_END_NAMESPACE

#endif // QWAYLANDCLIENT_H
