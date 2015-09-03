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

#ifndef QWAYLANDOUTPUTSPACE_P_H
#define QWAYLANDOUTPUTSPACE_P_H

#include <QtCore/private/qobject_p.h>

#include "qwaylandoutputspace.h"

#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandOutput>
#include <QtCompositor/private/qwaylandcompositor_p.h>
#include <QtCompositor/private/qwaylandoutput_p.h>

QT_BEGIN_NAMESPACE

class QWaylandOutputSpacePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandOutputSpace)

public:
    QWaylandOutputSpacePrivate(QWaylandCompositor *compositor)
        : QObjectPrivate()
        , compositor(compositor)
        , geometryConstraint(QWaylandOutputSpace::AutomaticBoundingRect)
    {

    }

    void adjustGeometry()
    {
        if (geometryConstraint != QWaylandOutputSpace::AutomaticBoundingRect)
            return;

        QRect completeRect;
        foreach(QWaylandOutput *output, outputs) {
            if (completeRect.isNull())
                completeRect = output->geometry();
            else
                completeRect = completeRect.united(output->geometry());
        }
        geometry = completeRect;
    }

    void addOutput(QWaylandOutput *output)
    {
        Q_Q(QWaylandOutputSpace);
        Q_ASSERT(output);
        Q_ASSERT(!outputs.contains(output));

        outputs.append(output);

        adjustGeometry();

        q->outputsChanged();
    }

    void removeOutput(QWaylandOutput *output)
    {
        Q_Q(QWaylandOutputSpace);
        Q_ASSERT(output);

        bool removed = outputs.removeOne(output);
        Q_ASSERT(removed);

        adjustGeometry();
        q->outputsChanged();
    }

    static QWaylandOutputSpacePrivate *get(QWaylandOutputSpace *outputSpace) { return outputSpace->d_func(); }

private:
    QWaylandCompositor *compositor;
    QRect geometry;
    QWaylandOutputSpace::GeometryConstraint geometryConstraint;
    QList<QWaylandOutput *> outputs;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDOUTPUTSPACE_P_H*/
