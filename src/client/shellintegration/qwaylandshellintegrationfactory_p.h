/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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
**
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

#ifndef QWAYLANDSHELLINTEGRATIONFACTORY_H
#define QWAYLANDSHELLINTEGRATIONFACTORY_H

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

#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandShellIntegration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellIntegrationFactory
{
public:
    static QStringList keys(const QString &pluginPath = QString());
    static QWaylandShellIntegration *create(const QString &name, QWaylandDisplay *display, const QStringList &args = QStringList(), const QString &pluginPath = QString());
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSHELLINTEGRATIONFACTORY_H
