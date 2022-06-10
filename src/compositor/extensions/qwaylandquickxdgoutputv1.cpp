// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
