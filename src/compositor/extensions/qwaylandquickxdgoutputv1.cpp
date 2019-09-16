/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QWaylandCompositor>
#include <QWaylandOutput>

#include "qwaylandquickxdgoutputv1.h"
#include "qwaylandxdgoutputv1_p.h"

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
