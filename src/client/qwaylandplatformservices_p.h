// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDPLATFORMSERVICES_H
#define QWAYLANDPLATFORMSERVICES_H

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

#include <QtGui/private/qgenericunixservices_p.h>

#include <QtWaylandClient/private/qwayland-qt-windowmanager.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLANDCLIENT_EXPORT QWaylandPlatformServices : public QGenericUnixServices
{
public:
    explicit QWaylandPlatformServices(QWaylandDisplay *waylandDisplay);

    bool openUrl(const QUrl &url) override;
    bool openDocument(const QUrl &url) override;
    QString portalWindowIdentifier(QWindow *window) override;

private:
    QWaylandDisplay *m_display;
};

QT_END_NAMESPACE

} // namespace QtWaylandClient

#endif // QWAYLANDPLATFORMSERVICES_H
