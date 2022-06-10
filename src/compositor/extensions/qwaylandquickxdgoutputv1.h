// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKXDGOUTPUT_V1
#define QWAYLANDQUICKXDGOUTPUT_V1

#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>
#include <QtWaylandCompositor/QWaylandXdgOutputV1>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickXdgOutputV1
        : public QWaylandXdgOutputV1
        , public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit QWaylandQuickXdgOutputV1();

protected:
    void classBegin() override {}
    void componentComplete() override;
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKXDGOUTPUT_V1
