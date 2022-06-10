// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTWAYLANDQMLINCLUDE_H
#define QTWAYLANDQMLINCLUDE_H

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
#include <QtCore/qglobal.h>
#include <QtWaylandCompositor/qtwaylandcompositor-config.h>

#if QT_CONFIG(wayland_compositor_quick)
#include <QtQml/qqml.h>
#else
#define QML_NAMED_ELEMENT(x)
#define QML_UNCREATABLE(x)
#define QML_ADDED_IN_VERSION(x, y)
#endif

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

#endif // QTWAYLANDQMLINCLUDE_H
