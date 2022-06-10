// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandCompositorExtensionPrivate : public QObjectPrivate
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
