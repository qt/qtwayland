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

            uint capabilities = MockCompositor::Seat::capability_pointer | MockCompositor::Seat::capability_touch;
            int version = 5;
            add<Seat>(capabilities, version);
        });
    }
};

class tst_seatv5 : public QObject, private SeatV5Compositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void bindsToSeat();

    // Pointer tests
    void createsPointer();
    void setsCursorOnEnter();
    void usesEnterSerial();
    void simpleAxis_data();
    void simpleAxis();
    void fingerScroll();
    void fingerScrollSlow();
    void continuousScroll();
    void wheelDiscreteScroll();

    // Touch tests
    void createsTouch();
    void singleTap();
    void singleTapFloat();
    void multiTouch();
    void multiTouchUpAndMotionFrame();
    void tapAndMoveInSameFrame();
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

        if (event->phase() != Qt::ScrollUpdate && event->phase() != Qt::NoScrollPhase) {
            // Shouldn't have deltas in the these phases
            QCOMPARE(event->angleDelta(), QPoint(0, 0));
            QCOMPARE(event->pixelDelta(), QPoint(0, 0));
        }

        // The axis vector of the event is already in surface space, so there is now way to tell
        // whether it is inverted or not.
        QCOMPARE(event->inverted(), false);

        // We didn't press any buttons
        QCOMPARE(event->buttons(), Qt::NoButton);

        m_events.append(Event{event});
    }
    struct Event // Because I didn't find a convenient way to copy it entirely
    {
        explicit Event() = default;
        explicit Event(const QWheelEvent *event)
            : phase(event->phase())
            , pixelDelta(event->pixelDelta())
            , angleDelta(event->angleDelta())
            , source(event->source())
        {
        }
        const Qt::ScrollPhase phase{};
        const QPoint pixelDelta;
        const QPoint angleDelta; // eights of a degree, positive is upwards, left
        const Qt::MouseEventSource source{};
    };
    QVector<Event> m_events;
};

void tst_seatv5::simpleAxis_data()
{
    QTest::addColumn<uint>("axis");
    QTest::addColumn<qreal>("value");
    QTest::addColumn<QPoint>("angleDelta");

    // Directions in regular windows/linux terms (no "natural" scrolling)
    QTest::newRow("down") << uint(Pointer::axis_vertical_scroll) << 1.0 << QPoint{0, -12};
    QTest::newRow("up") << uint(Pointer::axis_vertical_scroll) << -1.0 << QPoint{0, 12};
    QTest::newRow("left") << uint(Pointer::axis_horizontal_scroll) << 1.0 << QPoint{-12, 0};
    QTest::newRow("right") << uint(Pointer::axis_horizontal_scroll) << -1.0 << QPoint{12, 0};
    QTest::newRow("up big") << uint(Pointer::axis_vertical_scroll) << -10.0 << QPoint{0, 120};
}

void tst_seatv5::simpleAxis()
{
    QFETCH(uint, axis);
    QFETCH(qreal, value);
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
        QVERIFY(qAbs(e.angleDelta.x()) <= qAbs(e.angleDelta.y())); // Vertical scroll
//        QCOMPARE(e.angleDelta, angleDelta); // TODO: what should this be?
        QCOMPARE(e.pixelDelta, QPoint(0, -10));
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
        QVERIFY(qAbs(e.angleDelta.x()) > qAbs(e.angleDelta.y())); // Horizontal scroll
        QCOMPARE(e.pixelDelta, QPoint(-10, 0));
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
        QCOMPARE(e.pixelDelta, QPoint(-10, -10));
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
    QCOMPARE(accumulated.y(), -1);
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
        QVERIFY(qAbs(e.angleDelta.x()) <= qAbs(e.angleDelta.y())); // Vertical scroll
        // According to the docs the angle delta is in eights of a degree and most mice have
        // 1 click = 15 degrees. The angle delta should therefore be:
        // 15 degrees / (1/8 eights per degrees) = 120 eights of degrees.
        QCOMPARE(e.angleDelta, QPoint(0, -120));
        // Click scrolls are not continuous and should not have a pixel delta
        QCOMPARE(e.pixelDelta, QPoint(0, 0));
    }
}

void tst_seatv5::continuousScroll()
{
    WheelWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(xdgToplevel()->surface(), {32, 32});
        p->sendFrame(c);
        p->sendAxisSource(c, Pointer::axis_source_continuous);
        p->sendAxis(c, Pointer::axis_vertical_scroll, 10);
        p->sendAxis(c, Pointer::axis_horizontal_scroll, -5);
        p->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.phase, Qt::NoScrollPhase);
        QCOMPARE(e.pixelDelta, QPoint(5, -10));
        QCOMPARE(e.source, Qt::MouseEventSynthesizedBySystem); // touchpads are not wheels
    }
    // Sending axis_stop is not mandatory when axis source != finger
}

void tst_seatv5::createsTouch()
{
    QCOMPOSITOR_TRY_COMPARE(touch()->resourceMap().size(), 1);
    QCOMPOSITOR_TRY_COMPARE(touch()->resourceMap().first()->version(), 5);
}

class TouchWindow : public QRasterWindow {
public:
    TouchWindow()
    {
        resize(64, 64);
        show();
    }
    void touchEvent(QTouchEvent *event) override
    {
        QRasterWindow::touchEvent(event);
        m_events.append(Event{event});
    }
    struct Event // Because I didn't find a convenient way to copy it entirely
    {
        explicit Event() = default;
        explicit Event(const QTouchEvent *event)
            : type(event->type())
            , touchPointStates(event->touchPointStates())
            , touchPoints(event->touchPoints())
        {
        }
        const QEvent::Type type{};
        const Qt::TouchPointStates touchPointStates{};
        const QList<QTouchEvent::TouchPoint> touchPoints;
    };
    QVector<Event> m_events;
};

void tst_seatv5::singleTap()
{
    TouchWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *t = touch();
        auto *c = client();
        t->sendDown(xdgToplevel()->surface(), {32, 32}, 1);
        t->sendFrame(c);
        t->sendUp(c, 1);
        t->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchBegin);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints.first().pos(), QPointF(32-window.frameMargins().left(), 32-window.frameMargins().top()));
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchEnd);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints.first().pos(), QPointF(32-window.frameMargins().left(), 32-window.frameMargins().top()));
    }
}

void tst_seatv5::singleTapFloat()
{
    TouchWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *t = touch();
        auto *c = client();
        t->sendDown(xdgToplevel()->surface(), {32.75, 32.25}, 1);
        t->sendFrame(c);
        t->sendUp(c, 1);
        t->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchBegin);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints.first().pos(), QPointF(32.75-window.frameMargins().left(), 32.25-window.frameMargins().top()));
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchEnd);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints.first().pos(), QPointF(32.75-window.frameMargins().left(), 32.25-window.frameMargins().top()));
    }
}

void tst_seatv5::multiTouch()
{
    TouchWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *t = touch();
        auto *c = client();

        t->sendDown(xdgToplevel()->surface(), {32, 32}, 0);
        t->sendDown(xdgToplevel()->surface(), {48, 48}, 1);
        t->sendFrame(c);

        // Compositor event order should not change the order of the QTouchEvent::touchPoints()
        // See QTBUG-77014
        t->sendMotion(c, {49, 48}, 1);
        t->sendMotion(c, {33, 32}, 0);
        t->sendFrame(c);

        t->sendUp(c, 0);
        t->sendFrame(c);

        t->sendUp(c, 1);
        t->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchBegin);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints.length(), 2);

        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints[0].pos(), QPointF(32-window.frameMargins().left(), 32-window.frameMargins().top()));

        QCOMPARE(e.touchPoints[1].state(), Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints[1].pos(), QPointF(48-window.frameMargins().left(), 48-window.frameMargins().top()));
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchUpdate);
        QCOMPARE(e.touchPoints.length(), 2);

        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointMoved);
        QCOMPARE(e.touchPoints[0].pos(), QPointF(33-window.frameMargins().left(), 32-window.frameMargins().top()));

        QCOMPARE(e.touchPoints[1].state(), Qt::TouchPointState::TouchPointMoved);
        QCOMPARE(e.touchPoints[1].pos(), QPointF(49-window.frameMargins().left(), 48-window.frameMargins().top()));
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchUpdate);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointReleased | Qt::TouchPointState::TouchPointStationary);
        QCOMPARE(e.touchPoints.length(), 2);

        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints[0].pos(), QPointF(33-window.frameMargins().left(), 32-window.frameMargins().top()));

        QCOMPARE(e.touchPoints[1].state(), Qt::TouchPointState::TouchPointStationary);
        QCOMPARE(e.touchPoints[1].pos(), QPointF(49-window.frameMargins().left(), 48-window.frameMargins().top()));
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchEnd);
        QCOMPARE(e.touchPointStates, Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints[0].pos(), QPointF(49-window.frameMargins().left(), 48-window.frameMargins().top()));
    }
}

void tst_seatv5::multiTouchUpAndMotionFrame()
{
    TouchWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *t = touch();
        auto *c = client();

        t->sendDown(xdgToplevel()->surface(), {32, 32}, 0);
        t->sendDown(xdgToplevel()->surface(), {48, 48}, 1);
        t->sendFrame(c);

        // Sending an up event after a frame event, before any motion or down events used to
        // unnecessarily trigger a workaround for a bug in an old version of Weston. The workaround
        // would prematurely insert a fake frame event splitting the touch event up into two events.
        // However, this should only be needed on the up event for the very last touch point. So in
        // this test we verify that it doesn't unncecessarily break up the events.
        t->sendUp(c, 0);
        t->sendMotion(c, {49, 48}, 1);
        t->sendFrame(c);

        t->sendUp(c, 1);
        t->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchBegin);
        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointPressed);
        QCOMPARE(e.touchPoints[1].state(), Qt::TouchPointState::TouchPointPressed);
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchUpdate);
        QCOMPARE(e.touchPoints.length(), 2);
        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointReleased);
        QCOMPARE(e.touchPoints[1].state(), Qt::TouchPointState::TouchPointMoved);
    }
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchEnd);
        QCOMPARE(e.touchPoints.length(), 1);
        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointReleased);
    }
    QVERIFY(window.m_events.empty());
}

void tst_seatv5::tapAndMoveInSameFrame()
{
    TouchWindow window;
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([=] {
        auto *t = touch();
        auto *c = client();

        t->sendDown(xdgToplevel()->surface(), {32, 32}, 0);
        t->sendMotion(c, {33, 33}, 0);
        t->sendFrame(c);

        // Don't leave touch in a weird state
        t->sendUp(c, 0);
        t->sendFrame(c);
    });

    QTRY_VERIFY(!window.m_events.empty());
    {
        auto e = window.m_events.takeFirst();
        QCOMPARE(e.type, QEvent::TouchBegin);
        QCOMPARE(e.touchPoints.size(), 1);
        QCOMPARE(e.touchPoints[0].state(), Qt::TouchPointState::TouchPointPressed);
        // Position isn't that important, we just want to make sure we actually get the pressed event
    }

    // Make sure we eventually release
    QTRY_VERIFY(!window.m_events.empty());
    QTRY_COMPARE(window.m_events.last().touchPoints.first().state(), Qt::TouchPointState::TouchPointReleased);
}

QCOMPOSITOR_TEST_MAIN(tst_seatv5)
#include "tst_seatv5.moc"
