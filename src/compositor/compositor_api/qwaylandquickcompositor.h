// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKCOMPOSITOR_H
#define QWAYLANDQUICKCOMPOSITOR_H

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtQml/QQmlParserStatus>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QWaylandQuickCompositorPrivate;
class QWaylandView;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickCompositor : public QWaylandCompositor, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
public:
    QWaylandQuickCompositor(QObject *parent = nullptr);
    void create() override;

    void grabSurface(QWaylandSurfaceGrabber *grabber, const QWaylandBufferRef &buffer) override;

protected:
    void classBegin() override;
    void componentComplete() override;
};

QT_END_NAMESPACE

#endif
