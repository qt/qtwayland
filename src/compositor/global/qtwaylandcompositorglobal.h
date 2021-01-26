/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QWAYLANDCOMPOSITORGLOBAL_H
#define QWAYLANDCOMPOSITORGLOBAL_H

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

#include <QtGui/qtguiglobal.h>
#include <QtWaylandCompositor/qtwaylandcompositor-config.h>

QT_BEGIN_NAMESPACE

#if !defined(Q_WAYLAND_COMPOSITOR_EXPORT)
#  if defined(QT_SHARED) && defined(QT_BUILD_COMPOSITOR_LIB)
#    define Q_WAYLAND_COMPOSITOR_EXPORT Q_DECL_EXPORT
#  elif defined(QT_SHARED)
#    define Q_WAYLAND_COMPOSITOR_EXPORT Q_DECL_IMPORT
#  else
#    define Q_WAYLAND_COMPOSITOR_EXPORT
#  endif
#endif

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITORGLOBAL_H

