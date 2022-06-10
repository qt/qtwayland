// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDOUTPUTMODE_H
#define QWAYLANDOUTPUTMODE_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandOutputMode
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
