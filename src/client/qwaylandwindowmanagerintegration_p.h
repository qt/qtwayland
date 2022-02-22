// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDWINDOWMANAGERINTEGRATION_H
#define QWAYLANDWINDOWMANAGERINTEGRATION_H

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

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

#include <QtGui/private/qgenericunixservices_p.h>

#include <QtWaylandClient/private/qwayland-qt-windowmanager.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;

class QWaylandWindowManagerIntegrationPrivate;

class Q_WAYLANDCLIENT_EXPORT QWaylandWindowManagerIntegration : public QObject, public QGenericUnixServices, public QtWayland::qt_windowmanager
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandWindowManagerIntegration)
public:
    explicit QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay);
    ~QWaylandWindowManagerIntegration() override;

    bool openUrl(const QUrl &url) override;
    bool openDocument(const QUrl &url) override;
    QString portalWindowIdentifier(QWindow *window) override;

    bool showIsFullScreen() const;

private:
    static void wlHandleListenerGlobal(void *data, wl_registry *registry, uint32_t id,
                                       const QString &interface, uint32_t version);

    QScopedPointer<QWaylandWindowManagerIntegrationPrivate> d_ptr;

    void windowmanager_hints(int32_t showIsFullScreen) override;
    void windowmanager_quit() override;

    void openUrl_helper(const QUrl &url);
};

QT_END_NAMESPACE

}

#endif // QWAYLANDWINDOWMANAGERINTEGRATION_H
