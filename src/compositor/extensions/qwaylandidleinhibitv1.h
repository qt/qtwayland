// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDIDLEINHIBITV1_H
#define QWAYLANDIDLEINHIBITV1_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

class QWaylandIdleInhibitManagerV1Private;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandIdleInhibitManagerV1 : public QWaylandCompositorExtensionTemplate<QWaylandIdleInhibitManagerV1>
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
