// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDOUTPUTMODE_P_H
#define QWAYLANDOUTPUTMODE_P_H

#include <QtWaylandCompositor/QWaylandOutput>
#include <QtCore/private/qglobal_p.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandOutputModePrivate
{
public:
    QWaylandOutputModePrivate() {}

    QSize size;
    int refreshRate = 60000;
};

QT_END_NAMESPACE

#endif // QWAYLANDOUTPUTMODE_P_H
