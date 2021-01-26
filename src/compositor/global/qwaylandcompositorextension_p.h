/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QWAYLANDEXTENSION_P_H
#define QWAYLANDEXTENSION_P_H

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

#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/QWaylandCompositorExtension>

QT_BEGIN_NAMESPACE

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandCompositorExtensionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandCompositorExtension)

public:
    QWaylandCompositorExtensionPrivate()
    {
    }

    static QWaylandCompositorExtensionPrivate *get(QWaylandCompositorExtension *extension) { return extension->d_func(); }

    QWaylandObject *extension_container = nullptr;
    bool initialized = false;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDEXTENSION_P_H*/
