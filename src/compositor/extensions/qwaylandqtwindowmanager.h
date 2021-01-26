/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QWAYLANDQTWINDOWMANAGER_H
#define QWAYLANDQTWINDOWMANAGER_H

#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/QWaylandClient>

#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QWaylandQtWindowManagerPrivate;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQtWindowManager : public QWaylandCompositorExtensionTemplate<QWaylandQtWindowManager>
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
