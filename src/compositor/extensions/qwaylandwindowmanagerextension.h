/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WAYLANDWINDOWMANAGERINTEGRATION_H
#define WAYLANDWINDOWMANAGERINTEGRATION_H

#include <QtCompositor/qwaylandexport.h>
#include <QtCompositor/private/qwayland-server-windowmanager.h>

#include <QtCompositor/QWaylandExtension>
#include <QMap>

QT_BEGIN_NAMESPACE

namespace QtWayland {
    class Display;
}

class QWaylandCompositor;

class Q_COMPOSITOR_EXPORT WindowManagerServerIntegration : public QWaylandExtension, public QtWaylandServer::qt_windowmanager
{
    Q_OBJECT
public:
    explicit WindowManagerServerIntegration(QWaylandCompositor *compositor, QObject *parent = 0);
    ~WindowManagerServerIntegration();

    void initialize(QtWayland::Display *waylandDisplay);

    void setShowIsFullScreen(bool value);
    void sendQuitMessage(wl_client *client);

    const wl_interface *interface() const Q_DECL_OVERRIDE { return QtWaylandServer::qt_windowmanager::interface(); }
protected:
    void windowmanager_bind_resource(Resource *resource) Q_DECL_OVERRIDE;
    void windowmanager_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;
    void windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &url) Q_DECL_OVERRIDE;

private:
    bool m_showIsFullScreen;
    QWaylandCompositor *m_compositor;
    QMap<Resource*, QString> m_urls;
};

QT_END_NAMESPACE

#endif // WAYLANDWINDOWMANAGERINTEGRATION_H
