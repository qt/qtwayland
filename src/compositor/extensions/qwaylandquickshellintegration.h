// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKSHELLINTEGRATION_H
#define QWAYLANDQUICKSHELLINTEGRATION_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickShellIntegration : public QObject
{
    Q_OBJECT
public:
    QWaylandQuickShellIntegration(QObject *parent = nullptr);
    ~QWaylandQuickShellIntegration() override;
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSHELLINTEGRATION_H
