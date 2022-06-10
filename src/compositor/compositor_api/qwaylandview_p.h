// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSURFACEVIEW_P_H
#define QWAYLANDSURFACEVIEW_P_H

#include "qwaylandview.h"

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/private/qobject_p.h>

#include <QtWaylandCompositor/QWaylandBufferRef>

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

class QWaylandSurface;
class QWaylandOutput;

class QWaylandViewPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandView)
public:
    static QWaylandViewPrivate *get(QWaylandView *view) { return view->d_func(); }

    QWaylandViewPrivate()
    { }

    void markSurfaceAsDestroyed(QWaylandSurface *surface);
    void setSurface(QWaylandSurface *newSurface);
    void clearFrontBuffer();

    QObject *renderObject = nullptr;
    QWaylandSurface *surface = nullptr;
    QWaylandOutput *output = nullptr;
    QPointF requestedPos;
    QMutex bufferMutex;
    QWaylandBufferRef currentBuffer;
    QRegion currentDamage;
    QWaylandBufferRef nextBuffer;
    QRegion nextDamage;
    bool nextBufferCommitted = false;
    bool bufferLocked = false;
    bool broadcastRequestedPositionChanged = false;
    bool forceAdvanceSucceed = false;
    bool allowDiscardFrontBuffer = false;
    bool independentFrameCallback = false; //If frame callbacks are independent of the main quick scene graph
};

QT_END_NAMESPACE

#endif  /*QWAYLANDSURFACEVIEW_P_H*/
