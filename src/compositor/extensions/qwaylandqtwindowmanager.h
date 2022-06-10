// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTWINDOWMANAGER_H
#define QWAYLANDQTWINDOWMANAGER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandClient>

#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QWaylandQtWindowManagerPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtWindowManager : public QWaylandCompositorExtensionTemplate<QWaylandQtWindowManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandQtWindowManager)
    Q_PROPERTY(bool showIsFullScreen READ showIsFullScreen WRITE setShowIsFullScreen NOTIFY showIsFullScreenChanged)
public:
    QWaylandQtWindowManager();
    explicit QWaylandQtWindowManager(QWaylandCompositor *compositor);

    bool showIsFullScreen() const;
    void setShowIsFullScreen(bool value);

    void sendQuitMessage(QWaylandClient *client);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void showIsFullScreenChanged();
    void openUrl(QWaylandClient *client, const QUrl &url);
};

QT_END_NAMESPACE

#endif // QWAYLANDQTWINDOWMANAGER_H
