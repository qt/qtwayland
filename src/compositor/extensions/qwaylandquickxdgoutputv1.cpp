/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <QWaylandCompositor>
#include <QWaylandOutput>

#include "qwaylandquickxdgoutputv1.h"
#include "qwaylandxdgoutputv1_p.h"

QT_BEGIN_NAMESPACE

QWaylandQuickXdgOutputV1::QWaylandQuickXdgOutputV1()
    : QWaylandXdgOutputV1()
{
}

void QWaylandQuickXdgOutputV1::componentComplete()
{
    // Try to find the manager from the compositor extensions
    if (!manager()) {
        for (auto *p = parent(); p != nullptr; p = p->parent()) {
            if (auto *c = qobject_cast<QWaylandCompositor *>(p)) {
                for (auto *extension : c->extensions()) {
                    if (auto *m = qobject_cast<QWaylandXdgOutputManagerV1 *>(extension)) {
                        QWaylandXdgOutputV1Private::get(this)->setManager(m);
                        break;
                    }
                }
            }
        }
    }

    // Try to find the output from the parents
    if (!output()) {
        for (auto *p = parent(); p != nullptr; p = p->parent()) {
            if (auto *o = qobject_cast<QWaylandOutput *>(p)) {
                QWaylandXdgOutputV1Private::get(this)->setOutput(o);
                break;
            }
        }
    }
}

QT_END_NAMESPACE

#include "moc_qwaylandquickxdgoutputv1.cpp"
