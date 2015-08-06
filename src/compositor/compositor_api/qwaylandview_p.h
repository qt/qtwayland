/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSURFACEVIEW_P_H
#define QWAYLANDSURFACEVIEW_P_H

#include <QtCore/QPoint>
#include <QtCore/QMutex>

#include <QtCompositor/QWaylandBufferRef>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandOutput;

class QWaylandViewPrivate
{
    Q_DECLARE_PUBLIC(QWaylandView)
public:
    static QWaylandViewPrivate *get(QWaylandView *view);

    QWaylandViewPrivate(QWaylandView *view)
        : q_ptr(view)
        , surface(Q_NULLPTR)
        , output(Q_NULLPTR)
        , lockedBuffer(false)
    { }

    void markSurfaceAsDestroyed(QWaylandSurface *surface);

    bool shouldBroadcastRequestedPositionChanged() const
    {
        return broadcastRequestedPositionChanged && output && surface && surface->primaryOutput() == output;
    }

    QWaylandView *q_ptr;
    QWaylandSurface *surface;
    QWaylandOutput *output;
    QPointF requestedPos;
    QMutex bufferMutex;
    QWaylandBufferRef currentBuffer;
    QRegion currentDamage;
    QWaylandBufferRef nextBuffer;
    QRegion nextDamage;
    bool lockedBuffer;
    bool broadcastRequestedPositionChanged;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDSURFACEVIEW_P_H*/
