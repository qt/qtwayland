// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDPRESENTATIONTIME_P_H
#define QWAYLANDPRESENTATIONTIME_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QWaylandPresentationTimePrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandPresentationTime : public QWaylandCompositorExtensionTemplate<QWaylandPresentationTime>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandPresentationTime)
public:
    QWaylandPresentationTime();
    QWaylandPresentationTime(QWaylandCompositor *compositor);

    QWaylandCompositor *compositor() const;
    void initialize() override;

    Q_INVOKABLE void sendFeedback(QQuickWindow *window, quint64 sequence, quint64 tv_sec, quint32 tv_nsec);

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

signals:
    void presented(quint64 sequence, quint64 tv_sec, quint32 tv_nsec, quint32 refresh_nsec);
};

QT_END_NAMESPACE

#endif
