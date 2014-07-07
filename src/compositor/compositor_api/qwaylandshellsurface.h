/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

#include <QObject>

#include <QtCompositor/qwaylandexport.h>

#include "QtCompositor/qwaylandsurface.h"

QT_BEGIN_NAMESPACE

class QWaylandSurface;
class QWaylandShellSurfacePrivate;

class Q_COMPOSITOR_EXPORT QWaylandShellSurface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandShellSurface)
public:
    QWaylandShellSurface(QWaylandSurface *surface);
    virtual ~QWaylandShellSurface();

    // This could be const but it makes sense for it to not be, since
    // it's supposed to modify the surface
    virtual void requestSize(const QSize &size) = 0;
    virtual void ping(uint32_t serial) = 0;
    virtual QWaylandSurface::WindowType windowType() const = 0;
};

QT_END_NAMESPACE

#endif
