/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QWAYLANDCLIENTBUFFERINTEGRATIONFACTORY_H
#define QWAYLANDCLIENTBUFFERINTEGRATIONFACTORY_H

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

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandClientBufferIntegration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientBufferIntegrationFactory
{
public:
    static QStringList keys();
    static QWaylandClientBufferIntegration *create(const QString &name, const QStringList &args);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTBUFFERINTEGRATIONFACTORY_H
