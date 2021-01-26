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
****************************************************************************/

#ifndef QWAYLANDQUICKXDGOUTPUT_V1
#define QWAYLANDQUICKXDGOUTPUT_V1

#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>
#include <QtWaylandCompositor/QWaylandXdgOutputV1>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQuickXdgOutputV1
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
