/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QWAYLANDIDLEINHIBITV1_H
#define QWAYLANDIDLEINHIBITV1_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

class QWaylandIdleInhibitManagerV1Private;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandIdleInhibitManagerV1 : public QWaylandCompositorExtensionTemplate<QWaylandIdleInhibitManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandIdleInhibitManagerV1)
public:
    QWaylandIdleInhibitManagerV1();
    explicit QWaylandIdleInhibitManagerV1(QWaylandCompositor *compositor);
    ~QWaylandIdleInhibitManagerV1();

    void initialize() override;

    static const struct wl_interface *interface();
};

QT_END_NAMESPACE

#endif // QWAYLANDIDLEINHIBITV1_H
