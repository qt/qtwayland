// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QWaylandSeat>
#include <QList>

QT_BEGIN_NAMESPACE
class QInputEvent;
class QMouseEvent;
QT_END_NAMESPACE

class TestSeat : public QWaylandSeat
{
    Q_OBJECT
public:

    TestSeat(QWaylandCompositor *compositor, QWaylandSeat::CapabilityFlags caps);
    ~TestSeat() override;

    bool isOwner(QInputEvent *inputEvent) const override;

    QList<QMouseEvent *> createMouseEvents(int count);

    int queryCount() { return m_queryCount; }

private:
    mutable int m_queryCount;
    QList<QMouseEvent *> m_events;
};
