// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQtWindowManagerPrivate
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
