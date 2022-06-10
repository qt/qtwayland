// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQTSHELLCHROME_P_H
#define QWAYLANDQTSHELLCHROME_P_H

#include "qwaylandqtshell.h"

#include <QtCore/qpointer.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickdraghandler_p.h>

#include <QtWaylandCompositor/qwaylandquickshellsurfaceitem.h>

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

class QWaylandQtShellChromePrivate : public QQuickItemPrivate
{
public:
    void updateDecorationInteraction(quint8 flags, const QQuickHandlerPoint &centroid);
    QPointF constrainPoint(const QPointF &point) const;

    bool positionSet = false;
    bool automaticFrameMargins = true;
    QMargins explicitFrameMargins;

    uint currentState = Qt::WindowNoState;
    uint defaultFlags = Qt::Window
            | Qt::WindowMaximizeButtonHint
            | Qt::WindowMinimizeButtonHint
            | Qt::WindowCloseButtonHint;
    uint currentFlags = defaultFlags;
    QRect restoreGeometry = QRect(0, 0, 100, 100);
    QRect maximizedRect;
    QPointer<QWaylandQuickShellSurfaceItem> shellSurfaceItem;
    QPointer<QWaylandQtShellSurface> shellSurface;
    QPointer<QWaylandSurface> surface;
    QPointer<QWaylandQtShell> shell;

    enum class DecorationInteraction : quint8 {
        None = 0,
        WestBound = 1,
        EastBound = 2,
        NorthBound = 4,
        SouthBound = 8,
        TitleBar = 16
    };

    quint8 decorationInteraction = quint8(DecorationInteraction::None);
    QPointF decorationInteractionPosition;
    QRect decorationInteractionGeometry;

    QQuickItem *leftResizeHandle = nullptr;
    QQuickDragHandler *leftResizeHandleHandler = nullptr;
    QQuickDragHandler *rightResizeHandleHandler = nullptr;
    QQuickDragHandler *topResizeHandleHandler = nullptr;
    QQuickDragHandler *bottomResizeHandleHandler = nullptr;
    QQuickDragHandler *topLeftResizeHandleHandler = nullptr;
    QQuickDragHandler *topRightResizeHandleHandler = nullptr;
    QQuickDragHandler *bottomLeftResizeHandleHandler = nullptr;
    QQuickDragHandler *bottomRightResizeHandleHandler = nullptr;
    QQuickDragHandler *titleBarHandler = nullptr;

    QQuickItem *rightResizeHandle = nullptr;
    QQuickItem *topResizeHandle = nullptr;
    QQuickItem *bottomResizeHandle = nullptr;
    QQuickItem *topLeftResizeHandle = nullptr;
    QQuickItem *bottomLeftResizeHandle = nullptr;
    QQuickItem *topRightResizeHandle = nullptr;
    QQuickItem *bottomRightResizeHandle = nullptr;
    QQuickItem *titleBar = nullptr;
};

QT_END_NAMESPACE

#endif // QWAYLANDQTSHELLCHROME_P_H
