// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSHELLSURFACE_P_H
#define QWAYLANDSHELLSURFACE_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include "qwaylandshellsurface.h"

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellSurfacePrivate : public QWaylandCompositorExtensionPrivate
{
    Q_DECLARE_PUBLIC(QWaylandShellSurface)
public:
    bool modal = false;
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_P_H
