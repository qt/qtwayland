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

#include "qwaylandsurfaceview.h"
#include "qwaylandsurface.h"
#include "qwaylandsurface_p.h"
#include "qwaylandcompositor.h"
#include "qwaylandinput.h"

QT_BEGIN_NAMESPACE

class QWaylandSurfaceViewPrivate
{
public:
    QWaylandSurfaceViewPrivate()
        : surface(Q_NULLPTR)
    { }
    QWaylandSurface *surface;
    QPointF requestedPos;
};

QWaylandSurfaceView::QWaylandSurfaceView()
                   : d(new QWaylandSurfaceViewPrivate)
{
}

QWaylandSurfaceView::~QWaylandSurfaceView()
{
    if (d->surface) {
        QWaylandInputDevice *i = d->surface->compositor()->defaultInputDevice();
        if (i->mouseFocus() == this)
            i->setMouseFocus(Q_NULLPTR, QPointF());

        d->surface->destroy();
        d->surface->d_func()->views.removeOne(this);
        d->surface->deref();
    }

    delete d;
}

QWaylandSurface *QWaylandSurfaceView::surface() const
{
    return d->surface;
}

void QWaylandSurfaceView::setSurface(QWaylandSurface *newSurface)
{
    QWaylandSurface *oldSurface = d->surface;
    d->surface = newSurface;

    if (oldSurface)
        QWaylandSurfacePrivate::get(oldSurface)->derefView(this);

    if (newSurface)
        QWaylandSurfacePrivate::get(newSurface)->refView(this);

    waylandSurfaceChanged(newSurface, oldSurface);
}

QWaylandCompositor *QWaylandSurfaceView::compositor() const
{
    return d->surface ? d->surface->compositor() : 0;
}

void QWaylandSurfaceView::setRequestedPosition(const QPointF &pos)
{
    d->requestedPos = pos;
}

QPointF QWaylandSurfaceView::requestedPosition() const
{
    return d->requestedPos;
}

QPointF QWaylandSurfaceView::pos() const
{
    return d->requestedPos;
}

void QWaylandSurfaceView::waylandSurfaceChanged(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_UNUSED(newSurface);
    Q_UNUSED(oldSurface);
}

QT_END_NAMESPACE
