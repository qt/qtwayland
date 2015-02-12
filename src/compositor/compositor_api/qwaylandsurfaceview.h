/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSURFACEVIEW_H
#define QWAYLANDSURFACEVIEW_H

#include <QPointF>

#include <QtCompositor/qwaylandexport.h>

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandCompositor;

class Q_COMPOSITOR_EXPORT QWaylandSurfaceView
{
public:
    QWaylandSurfaceView(QWaylandSurface *surface);
    virtual ~QWaylandSurfaceView();

    QWaylandCompositor *compositor() const;
    QWaylandSurface *surface() const;

    virtual void setPos(const QPointF &pos);
    virtual QPointF pos() const;

private:
    class QWaylandSurfaceViewPrivate *const d;
    friend class QWaylandSurfaceViewPrivate;
};

QT_END_NAMESPACE

#endif
