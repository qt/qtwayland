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

#include "qwaylandoutputspace.h"
#include "qwaylandoutputspace_p.h"
#include "qwaylandcompositor.h"
#include <QtWaylandCompositor/QWaylandView>
#include <QtWaylandCompositor/QWaylandOutput>

QT_BEGIN_NAMESPACE

QWaylandOutputSpace::QWaylandOutputSpace(QWaylandCompositor *compositor)
    : QObject(*new QWaylandOutputSpacePrivate(compositor), compositor)
{

}

QWaylandCompositor *QWaylandOutputSpace::compositor() const
{
    Q_D(const QWaylandOutputSpace);
    return d->compositor;
}

QRect QWaylandOutputSpace::geometry() const
{
    Q_D(const QWaylandOutputSpace);
    return d->geometry;
}

void QWaylandOutputSpace::setGeometry(const QRect &geometry)
{
    Q_D(QWaylandOutputSpace);
    if (d->geometry == geometry || d->geometryConstraint == QWaylandOutputSpace::AutomaticBoundingRect)
        return;
    d->geometry = geometry;
    emit geometryChanged();
}

void QWaylandOutputSpace::setGeometryConstraint(QWaylandOutputSpace::GeometryConstraint geometryConstraint)
{
    Q_D(QWaylandOutputSpace);
    if (d->geometryConstraint == geometryConstraint)
        return;
    d->geometryConstraint = geometryConstraint;
    emit geometryConstraintChanged();
}

QWaylandOutputSpace::GeometryConstraint QWaylandOutputSpace::geometryConstraint() const
{
    Q_D(const QWaylandOutputSpace);
    return d->geometryConstraint;
}

QWaylandOutput *QWaylandOutputSpace::output(QWindow *window) const
{
    Q_D(const QWaylandOutputSpace);
    foreach (QWaylandOutput *output, d->outputs) {
        if (output->window() == window)
            return output;
    }
    return Q_NULLPTR;
}

void QWaylandOutputSpace::setDefaultOutput(QWaylandOutput *output)
{
    Q_D(QWaylandOutputSpace);
    if (d->outputs.isEmpty() || d->outputs.first() == output)
        return;

    if (d->outputs.removeOne(output)) {
        d->outputs.prepend(output);
        defaultOutputChanged();
    }
}

QWaylandOutput *QWaylandOutputSpace::defaultOutput() const
{
    Q_D(const QWaylandOutputSpace);
    if (d->outputs.isEmpty())
        return Q_NULLPTR;

    return d->outputs.first();
}

QList<QWaylandOutput *>QWaylandOutputSpace::outputs() const
{
    Q_D(const QWaylandOutputSpace);
    return d->outputs;
}

QList<QWaylandOutput *>QWaylandOutputSpace::outputs(const QPoint &point) const
{
    Q_D(const QWaylandOutputSpace);
    QList<QWaylandOutput *> retOutputs;
    foreach (QWaylandOutput *output, d->outputs) {
        if (output->geometry().contains(point))
            retOutputs.append(output);
    }
    return retOutputs;
}

QT_END_NAMESPACE
