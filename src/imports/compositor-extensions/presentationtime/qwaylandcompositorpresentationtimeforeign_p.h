// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCOMPOSITORPRESENTATIONTIMEFOREIGN_H
#define QWAYLANDCOMPOSITORPRESENTATIONTIMEFOREIGN_H

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
#include <QtWaylandCompositor/private/qwaylandpresentationtime_p.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_ELEMENT(QWaylandPresentationTime, PresentationTime, 1, 0)

QT_END_NAMESPACE

#endif
