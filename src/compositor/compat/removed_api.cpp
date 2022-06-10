// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#define QT_WAYLANDCOMPOSITOR_BUILD_REMOVED_API

#include "qtwaylandcompositorglobal.h"

QT_USE_NAMESPACE

#if QT_WAYLANDCOMPOSITOR_REMOVED_SINCE(6, 3)

#include "qwaylandbufferref.h"

bool QWaylandBufferRef::operator==(const QWaylandBufferRef &other)
{
    return std::as_const(*this) == other;
}

bool QWaylandBufferRef::operator!=(const QWaylandBufferRef &other)
{
    return std::as_const(*this) != other;
}

#endif // QT_WAYLANDCOMPOSITOR_REMOVED_SINCE(6, 3)

#if QT_WAYLANDCOMPOSITOR_REMOVED_SINCE(6, 4)

// #include "qotherheader.h"
// // implement removed functions from qotherheader.h
// order alphabetically

#endif // QT_WAYLANDCOMPOSITOR_REMOVED_SINCE(6, 4)
