// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKOUTPUT_H
#define QWAYLANDQUICKOUTPUT_H

#include <QtQuick/QQuickWindow>
#include <QtWaylandCompositor/qwaylandoutput.h>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

QT_BEGIN_NAMESPACE

class QWaylandQuickCompositor;
class QQuickWindow;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickOutput : public QWaylandOutput, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandQuickOutput)
    Q_PROPERTY(bool automaticFrameCallback READ automaticFrameCallback WRITE setAutomaticFrameCallback NOTIFY automaticFrameCallbackChanged)
    QML_NAMED_ELEMENT(WaylandOutput)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QWaylandQuickOutput();
    QWaylandQuickOutput(QWaylandCompositor *compositor, QWindow *window);

    void update() override;

    bool automaticFrameCallback() const;
    void setAutomaticFrameCallback(bool automatic);

    QQuickItem *pickClickableItem(const QPointF &position);

public Q_SLOTS:
    void updateStarted();

Q_SIGNALS:
    void automaticFrameCallbackChanged();

protected:
    void initialize() override;
    void classBegin() override;
    void componentComplete() override;

private:
    void doFrameCallbacks();

    bool m_updateScheduled = false;
    bool m_automaticFrameCallback = true;
};

QT_END_NAMESPACE

#endif
