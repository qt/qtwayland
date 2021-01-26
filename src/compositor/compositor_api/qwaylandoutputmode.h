/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QWAYLANDOUTPUTMODE_H
#define QWAYLANDOUTPUTMODE_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandOutputMode
{
public:
    explicit QWaylandOutputMode();
    QWaylandOutputMode(const QSize &size, int refreshRate);
    QWaylandOutputMode(const QWaylandOutputMode &other);
    ~QWaylandOutputMode();

    QWaylandOutputMode &operator=(const QWaylandOutputMode &other);
    bool operator==(const QWaylandOutputMode &other) const;
    bool operator!=(const QWaylandOutputMode &other) const;

    bool isValid() const;

    QSize size() const;
    int refreshRate() const;

private:
    class QWaylandOutputModePrivate *const d;
    friend class QWaylandOutputPrivate;

    void setSize(const QSize &size);
};
Q_DECLARE_TYPEINFO(QWaylandOutputMode, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

#endif // QWAYLANDOUTPUTMODE_H
