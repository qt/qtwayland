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

#ifndef QWAYLANDQTWINDOWMANAGER_P_H
#define QWAYLANDQTWINDOWMANAGER_P_H

#include <QtCore/QMap>

#include <QtWaylandCompositor/QWaylandQtWindowManager>
#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/private/qwayland-server-qt-windowmanager.h>

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

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandQtWindowManagerPrivate
        : public QWaylandCompositorExtensionPrivate
        , public QtWaylandServer::qt_windowmanager
{
    Q_DECLARE_PUBLIC(QWaylandQtWindowManager)
public:
    QWaylandQtWindowManagerPrivate();

protected:
    void windowmanager_bind_resource(Resource *resource) override;
    void windowmanager_destroy_resource(Resource *resource) override;
    void windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &url) override;

private:
    bool showIsFullScreen = false;
    QMap<Resource*, QString> urls;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTWINDOWMANAGER_P_H
