/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QWAYLANDSURFACEVIEW_H
#define QWAYLANDSURFACEVIEW_H

#include <QPointF>

#include <QtCompositor/QWaylandBufferRef>
#include <QtCompositor/qwaylandexport.h>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandCompositor;
class QWaylandViewPrivate;

class Q_COMPOSITOR_EXPORT QWaylandView
{
    Q_DECLARE_PRIVATE(QWaylandView)
public:
    QWaylandView();
    virtual ~QWaylandView();

    QWaylandCompositor *compositor() const;

    QWaylandSurface *surface() const;
    void setSurface(QWaylandSurface *surface);

    QWaylandOutput *output() const;
    void setOutput(QWaylandOutput *output);

    virtual void setRequestedPosition(const QPointF &pos);
    virtual QPointF requestedPosition() const;
    virtual QPointF pos() const;

    virtual void attach(const QWaylandBufferRef &ref, const QRegion &damage);
    virtual bool advance();
    virtual QWaylandBufferRef currentBuffer();
    virtual QRegion currentDamage();

    bool lockedBuffer() const;
    void setLockedBuffer(bool locked);

    bool broadcastRequestedPositionChanged() const;
    void setBroadcastRequestedPositionChanged(bool broadcast);

    struct wl_resource *surfaceResource() const;
protected:
    virtual void waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface);
    virtual void waylandSurfaceDestroyed();
    virtual void waylandOutputChanged(QWaylandOutput *newOutput, QWaylandOutput *oldOutput);

private:
    QScopedPointer<QWaylandViewPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif
