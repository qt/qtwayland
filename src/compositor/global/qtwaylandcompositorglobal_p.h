// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCOMPOSITORGLOBAL_P_H
#define QWAYLANDCOMPOSITORGLOBAL_P_H

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

#include <QtWaylandGlobal/private/qtwaylandglobal-config_p.h>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtGui/private/qtguiglobal_p.h>
#include <QtWaylandCompositor/private/qtwaylandcompositor-config_p.h>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(wayland_compositor_quick)
void Q_WAYLANDCOMPOSITOR_EXPORT qml_register_types_QtWayland_Compositor();
#endif

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITORGLOBAL_P_H

