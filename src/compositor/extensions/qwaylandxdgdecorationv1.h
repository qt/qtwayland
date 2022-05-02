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

#ifndef QWAYLANDXDGDECORATIONV1_H
#define QWAYLANDXDGDECORATIONV1_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

QT_BEGIN_NAMESPACE

class QWaylandXdgDecorationManagerV1Private;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgDecorationManagerV1 : public QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgDecorationManagerV1)
    Q_PROPERTY(QWaylandXdgToplevel::DecorationMode preferredMode READ preferredMode WRITE setPreferredMode NOTIFY preferredModeChanged)

public:
    explicit QWaylandXdgDecorationManagerV1();

    void initialize() override;

    QWaylandXdgToplevel::DecorationMode preferredMode() const;
    void setPreferredMode(QWaylandXdgToplevel::DecorationMode preferredMode);

    static const struct wl_interface *interface();

Q_SIGNALS:
    void preferredModeChanged();
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGDECORATIONV1_H
