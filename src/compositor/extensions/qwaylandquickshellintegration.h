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

#ifndef QWAYLANDQUICKSHELLINTEGRATION_H
#define QWAYLANDQUICKSHELLINTEGRATION_H

#include <QtCore/QObject>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickShellIntegration : public QObject
{
    Q_OBJECT
public:
    QWaylandQuickShellIntegration(QObject *parent = nullptr);
    ~QWaylandQuickShellIntegration() override;
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSHELLINTEGRATION_H
