// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testseat.h"
#include <QMouseEvent>

TestSeat::TestSeat(QWaylandCompositor *compositor, QWaylandSeat::CapabilityFlags caps)
    : QWaylandSeat(compositor, caps)
{
    m_queryCount = 0;
}

TestSeat::~TestSeat()
{
}

bool TestSeat::isOwner(QInputEvent *event) const
{
    m_queryCount++;
    QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);
    return m_events.contains(me);
}

QList<QMouseEvent *> TestSeat::createMouseEvents(int count)
{
    for (int i = 0; i < count; i++) {
        m_events.append(new QMouseEvent(QEvent::MouseMove, QPointF(10 + i, 10 + i),
                                        QPointF(10 + i, 10 + i), Qt::NoButton, Qt::NoButton, Qt::NoModifier));
    }
    return m_events;
}
