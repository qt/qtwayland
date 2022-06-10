// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGDECORATIONV1_H
#define QWAYLANDXDGDECORATIONV1_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandXdgToplevel>

QT_BEGIN_NAMESPACE

class QWaylandXdgDecorationManagerV1Private;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgDecorationManagerV1 : public QWaylandCompositorExtensionTemplate<QWaylandXdgDecorationManagerV1>
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
