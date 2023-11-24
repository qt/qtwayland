// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDWLSHELLFOREIGN_H
#define QWAYLANDWLSHELLFOREIGN_H

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

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include <QtWaylandCompositor/qwaylandwlshell.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandWlShell, WlShell, 1, 0)

struct QWaylandWlShellForeign {
    Q_GADGET
    QML_FOREIGN(QWaylandWlShell)
    QML_NAMED_ELEMENT(WlShellSurface)
    QML_ADDED_IN_VERSION(1, 0)
};

QT_END_NAMESPACE

#endif
