/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QWAYLANDVIEWPORTER_H
#define QWAYLANDVIEWPORTER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

class QWaylandViewporterPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandViewporter
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
