// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKSURFACE_P_H
#define QWAYLANDQUICKSURFACE_P_H

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

#include <QtWaylandCompositor/private/qwaylandsurface_p.h>

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickSurfacePrivate : public QWaylandSurfacePrivate
{
    Q_DECLARE_PUBLIC(QWaylandQuickSurface)
public:
    QWaylandQuickSurfacePrivate()
    {
    }

    ~QWaylandQuickSurfacePrivate() override
    {
    }

    bool useTextureAlpha = true;
    bool clientRenderingEnabled = true;
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKSURFACE_P_H
