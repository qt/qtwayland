// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCOMPOSITORXDGSHELLFOREIGN_H
#define QWAYLANDCOMPOSITORXDGSHELLFOREIGN_H

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

#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandXdgDecorationManagerV1>
#include <QtWaylandCompositor/QWaylandQuickXdgOutputV1>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandXdgShell, XdgShell, 1, 3)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandXdgDecorationManagerV1,
                                                   XdgDecorationManagerV1, 1, 3)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandXdgOutputManagerV1, XdgOutputManagerV1,
                                                   1, 14)

struct QWaylandXdgSurfaceForeign {
    Q_GADGET
    QML_FOREIGN(QWaylandXdgSurface)
    QML_NAMED_ELEMENT(XdgSurface)
    QML_ADDED_IN_VERSION(1, 3)
};

struct QWaylandXdgTopLevelForeign {
    Q_GADGET
    QML_FOREIGN(QWaylandXdgToplevel)
    QML_NAMED_ELEMENT(XdgToplevel)
    QML_ADDED_IN_VERSION(1, 3)
    QML_UNCREATABLE("Cannot create instance of XdgShellToplevel")
};

struct QWaylandXdgPopupForeign {
    Q_GADGET
    QML_FOREIGN(QWaylandXdgPopup)
    QML_NAMED_ELEMENT(XdgPopup)
    QML_ADDED_IN_VERSION(1, 3)
    QML_UNCREATABLE("Cannot create instance of XdgShellPopup")
};

struct QWaylandQuickXdgOutputV1Foreign {
    Q_GADGET
    QML_FOREIGN(QWaylandQuickXdgOutputV1)
    QML_NAMED_ELEMENT(XdgOutputV1)
    QML_ADDED_IN_VERSION(1, 14)
};

QT_END_NAMESPACE

#endif
