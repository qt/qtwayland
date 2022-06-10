// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDVIEWPORTER_H
#define QWAYLANDVIEWPORTER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

class QWaylandViewporterPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandViewporter
        : public QWaylandCompositorExtensionTemplate<QWaylandViewporter>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandViewporter)

public:
    explicit QWaylandViewporter();
    explicit QWaylandViewporter(QWaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
};

QT_END_NAMESPACE

#endif // QWAYLANDVIEWPORTER_H
