// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKSHELLSURFACEITEM_P_H
#define QWAYLANDQUICKSHELLSURFACEITEM_P_H

#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>
#include <QtWaylandCompositor/QWaylandQuickShellIntegration>
#include <QtWaylandCompositor/private/qwaylandquickitem_p.h>
#include <QtCore/QBasicTimer>

#include <functional>

QT_BEGIN_NAMESPACE

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

class QWaylandShellSurface;
class QWaylandQuickShellSurfaceItem;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickShellSurfaceItemPrivate : public QWaylandQuickItemPrivate
{
    Q_DECLARE_PUBLIC(QWaylandQuickShellSurfaceItem)
public:
    QWaylandQuickShellSurfaceItemPrivate() {}
    QWaylandQuickShellSurfaceItem *maybeCreateAutoPopup(QWaylandShellSurface* shellSurface);
    static QWaylandQuickShellSurfaceItemPrivate *get(QWaylandQuickShellSurfaceItem *item) { return item->d_func(); }

    void raise() override;
    void lower() override;

    QWaylandQuickShellIntegration *m_shellIntegration = nullptr;
    QWaylandShellSurface *m_shellSurface = nullptr;
    QQuickItem *m_moveItem = nullptr;
    bool m_autoCreatePopupItems = true;
    bool staysOnTop = false;
    bool staysOnBottom = false;
};

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickShellEventFilter : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void()> CallbackFunction;
    static void startFilter(QWaylandClient *client, CallbackFunction closePopupCallback);
    static void cancelFilter();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void stopFilter();

    QWaylandQuickShellEventFilter(QObject *parent = nullptr);
    bool eventFilter(QObject *, QEvent *) override;
    bool eventFilterInstalled = false;
    bool waitForRelease = false;
    QPointer<QWaylandClient> client;
    CallbackFunction closePopups = nullptr;
    QBasicTimer mousePressTimeout;
    static QWaylandQuickShellEventFilter *self;
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSHELLSURFACEITEM_P_H
