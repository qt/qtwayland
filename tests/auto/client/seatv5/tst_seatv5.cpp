/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockcompositor.h"
#include <QtGui/QRasterWindow>
#include <QtGui/QOpenGLWindow>

using namespace MockCompositor;

class SeatV5Compositor : public DefaultCompositor {
public:
    explicit SeatV5Compositor()
    {
        exec([this] {
            m_config.autoConfigure = true;

            removeAll<Seat>();

            uint capabilities = MockCompositor::Seat::capability_pointer;
            int version = 5;
            add<Seat>(capabilities, version);
        });
    }

    Pointer *pointer()
    {
        auto *seat = get<Seat>();
        Q_ASSERT(seat);
        return seat->m_pointer;
    }
};

class tst_seatv5 : public QObject, private SeatV5Compositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void bindsToSeat();
    void createsPointer();
    void setsCursorOnEnter();
    void usesEnterSerial();
    void simpleAxis_data();
    void simpleAxis();
    void fingerScroll();
    void fingerScrollSlow();
    void wheelDiscreteScroll();
};

void tst_seatv5::bindsToSeat()
{
    QCOMPOSITOR_COMPARE(get<Seat>()->resourceMap().size(), 1);
    QCOMPOSITOR_COMPARE(get<Seat>()->resourceMap().first()->version(), 5);
}

void tst_seatv5::createsPointer()
{
    QCOMPOSITOR_TRY_COMPARE(pointer()->resourceMap().size(), 1);
    QCOMPOSITOR_TRY_COMPARE(pointer()->resourceMap().first()->version(), 5);
}

void tst_seatv5::setsCursorOnEnter()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *surface = xdgSurface()->m_surface;
        pointer()->sendEnter(surface, {0, 0});
        pointer()->sendFrame(surface->resource()->client());
    });

    QCOMPOSITOR_TRY_VERIFY(pointer()->cursorSurface());
}

void tst_seatv5::usesEnterSerial()
{
    QSignalSpy setCursorSpy(exec([=] { return pointer(); }), &Pointer::setCursor);
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    uint enterSerial = exec([=] {
        return pointer()->sendEnter(xdgSurface()->m_surface, {0, 0});
    });
    QCOMPOSITOR_TRY_VERIFY(pointer()->cursorSurface());

    QTRY_COMPARE(setCursorSpy.count(), 1);
    QCOMPARE(setCursorSpy.takeFirst().at(0).toUInt(), enterSerial);
}

class WheelWindow : QRasterWindow {
public:
    WheelWindow()
    {
        resize(64, 64);
        show();
    }
    void wheelEvent(QWheelEvent *event) override
    {
        QRasterWindow::wheelEvent(event);
//        qDebug() << event << "angleDelta" << event->angleDelta() << "pixelDelta" << event->pixelDelta();

        if (event->phase() == Qt::ScrollUpdate || event->phase() == Qt::NoScrollPhase) {
            // Angle delta should always be provided (says docs, but QPA sends compatibility events
            // for Qt4 with zero angleDelta, and with a delta)
            QVERIFY(!event->angleDelta().isNull() || event->delta());
        } else {
            // Shouldn't have deltas in the other phases
            QCOMPARE(event->angleDelta(), QPoint(0, 0));
            QCOMPARE(event->pixelDelta(), QPoint(0, 0));
        }

        // The axis vector of the event is already in surface space, so there is now way to tell
        // whether it is inverted or not.
        QCOMPARE(event->inverted(), false);

        // We didn't press any buttons
        QCOMPARE(event->buttons(), Qt::NoButton);

        if (!event->angleDelta().isNull()) {
            if (event->orientation() == Qt::Horizontal)
                QCOMPARE(event->delta(), event->angleDelta().x());
            else
                QCOMPARE(event->delta(), event->angleDelta().y());
        }

        m_events.append(Event{event});
    }
    struct Event // Because I didn't find a convenient way to copy it entirely
    {
        explicit Event() = default;
        explicit Event(const QWheelEvent *event)
            : phase(event->phase())
            , pixelDelta(event->pixelDelta())
            , angleDelta(event->angleDelta())
            , orientation(event->orientation())
            , source(event->source())
        {
        }
        const Qt::ScrollPhase phase{};
        const QPoint pixelDelta;
        const QPoint angleDelta; // eights of a degree, positive is upwards, left
        const Qt::Orientation orientation{};
        const Qt::MouseEventSource source{};
    };
    QVector<Event> m_events;
};

void tst_seatv5::simpleAxis_data()
{
    QTest::addColumn<uint>("axis");
    QTest::addColumn<qreal>("value");
    QTest::addColumn<Qt::Orientation>("orientation");
    QTest::addColumn<QPoint>("angleDelta");

    // Directions in regular windows/linux terms (no "natural" scrolling)
    QTest::newRow("down") << uint(Pointer::axis_vertical_scroll) << 1.0 << Qt::Vertical << QPoint{0, -12};
    QTest::newRow("up") << uint(Pointer::axis_vertical_scroll) << -1.0 << Qt::Vertical << QPoint{0, 12};
    QTest::newRow("left") << uint(Pointer::axis_horizontal_scroll) << 1.0 << Qt::Horizontal << QPoint{-12, 0};
    QTest::newRow("right") << uint(Pointer::axis_horizontal_scroll) << -1.0 << Qt::Horizontal << QPoint{12, 0};
    QTest::newRow("up big") << uint(Pointer::axis_vertical_scroll) << -10.0 << Qt::Vertical << QPoint{0, 120};
}

void tst_seatv5::simpleAxis()
{
    QFETCH(uint, axis);
    QFETCH(qreal, value);
    QFETCH(Qt::Orientation, orientation);
    QFETCH(QPoint, angleDelta);

    WheelWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *p = pointer();
        p->sendEnter(xdgToplevel()->surface(), {32, 32});
        p->sendFrame(client());
        p->sendAxis(
            client(),
            Pointer::axis(axis),
            value // Length of vector in surface-local space. i.e. positive is downwards
        );
        p->sendFrame(client());
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::NoScrollPhase);
        // Pixel delta should only be set if we know it's a high-res input device (which we don't)
        QCOMPARE(e.pixelDelta, QPoint(0, 0));
        // There has been no information about what created the event.
        // Documentation says not synthesized is appropriate in such cases
        QCOMPARE(e.source, Qt::MouseEventNotSynthesized);
        QCOMPARE(e.orientation, orientation);
        QCOMPARE(e.angleDelta, angleDelta);
    }

    // Sending axis_stop is not mandatory when axis source != finger
}

void tst_seatv5::fingerScroll()
{
    WheelWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(xdgToplevel()->surface(), {32, 32});
        p->sendFrame(c);
        p->sendAxisSource(c, Pointer::axis_source_finger);
        p->sendAxis(c, Pointer::axis_vertical_scroll, 10);
        p->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::ScrollBegin);
        QCOMPARE(e.angleDelta, QPoint());
        QCOMPARE(e.pixelDelta, QPoint());
    }

    QTRY_VERIFY(!window.m_events.empty());
    // For some reason we send two ScrollBegins, one for each direction, not sure if this is really
    // necessary, (could be removed from QtBase, hence the conditional below.
    if (window.m_events.first().phase == Qt::ScrollBegin) {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.angleDelta, QPoint());
        QCOMPARE(e.pixelDelta, QPoint());
    }

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::ScrollUpdate);
        QCOMPARE(e.orientation, Qt::Vertical);
//        QCOMPARE(e.angleDelta, angleDelta); // TODO: what should this be?
        QCOMPARE(e.pixelDelta, QPoint(0, 10));
        QCOMPARE(e.source, Qt::MouseEventSynthesizedBySystem); // A finger is not a wheel
    }

    QTRY_VERIFY(window.m_events.empty());

    // Scroll horizontally as well
    exec([=] {
        pointer()->sendAxisSource(client(), Pointer::axis_source_finger);
        pointer()->sendAxis(client(), Pointer::axis_horizontal_scroll, 10);
        pointer()->sendFrame(client());
    });
    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::ScrollUpdate);
        QCOMPARE(e.orientation, Qt::Horizontal);
        QCOMPARE(e.pixelDelta, QPoint(10, 0));
        QCOMPARE(e.source, Qt::MouseEventSynthesizedBySystem); // A finger is not a wheel
    }

    // Scroll diagonally
    exec([=] {
        pointer()->sendAxisSource(client(), Pointer::axis_source_finger);
        pointer()->sendAxis(client(), Pointer::axis_horizontal_scroll, 10);
        pointer()->sendAxis(client(), Pointer::axis_vertical_scroll, 10);
        pointer()->sendFrame(client());
    });
    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::ScrollUpdate);
        QCOMPARE(e.pixelDelta, QPoint(10, 10));
        QCOMPARE(e.source, Qt::MouseEventSynthesizedBySystem); // A finger is not a wheel
    }

    // For diagonal events, Qt sends an additional compatibility ScrollUpdate event
    if (window.m_events.first().phase == Qt::ScrollUpdate) {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.angleDelta, QPoint());
        QCOMPARE(e.pixelDelta, QPoint());
    }

    QVERIFY(window.m_events.empty());

    // Sending axis_stop is mandatory when axis source == finger
    exec([=] {
        pointer()->sendAxisStop(client(), Pointer::axis_vertical_scroll);
        pointer()->sendFrame(client());
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::ScrollEnd);
    }
}


void tst_seatv5::fingerScrollSlow()
{
    WheelWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(xdgToplevel()->surface(), {32, 32});
        p->sendFrame(c);
        // Send 10 really small updates
        for (int i = 0; i < 10; ++i) {
            p->sendAxisSource(c, Pointer::axis_source_finger);
            p->sendAxis(c, Pointer::axis_vertical_scroll, 0.1);
            p->sendFrame(c);
        }
        p->sendAxisStop(c, Pointer::axis_vertical_scroll);
        p->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    QPoint accumulated;
    while (window.m_events.first().phase != Qt::ScrollEnd) {
        auto e = window.m_events.takeFirst();
        accumulated += e.pixelDelta;
        QTRY_VERIFY(!window.m_events.empty());
    }
    QCOMPARE(accumulated.y(), 1);
}
void tst_seatv5::wheelDiscreteScroll()
{
    WheelWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(xdgToplevel()->surface(), {32, 32});
        p->sendFrame(c);
        p->sendAxisSource(c, Pointer::axis_source_wheel);
        p->sendAxisDiscrete(c, Pointer::axis_vertical_scroll, 1); // 1 click downwards
        p->sendAxis(c, Pointer::axis_vertical_scroll, 1.0);
        p->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::NoScrollPhase);
        QCOMPARE(e.orientation, Qt::Vertical);
        // According to the docs the angle delta is in eights of a degree and most mice have
        // 1 click = 15 degrees. The angle delta should therefore be:
        // 15 degrees / (1/8 eights per degrees) = 120 eights of degrees.
        QCOMPARE(e.angleDelta, QPoint(0, -120));
        // Click scrolls are not continuous and should not have a pixel delta
        QCOMPARE(e.pixelDelta, QPoint(0, 0));
    }
}

QCOMPOSITOR_TEST_MAIN(tst_seatv5)
#include "tst_seatv5.moc"
