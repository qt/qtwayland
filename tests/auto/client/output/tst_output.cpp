/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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
#include <QtGui/QScreen>
#include <QtGui/QRasterWindow>

using namespace MockCompositor;

class tst_output : public QObject, private DefaultCompositor
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
        m_config.autoConfigure = true;
        m_config.autoEnter = false;
    }
    void cleanup()
    {
        QCOMPOSITOR_COMPARE(getAll<Output>().size(), 1); // Only the default output should be left
        QTRY_COMPARE(QGuiApplication::screens().size(), 1);
        QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage()));
    }
    void primaryScreen();
    void secondaryHiDpiScreen();
    void addScreenWithGeometryChange();
    void windowScreens();
    void removePrimaryScreen();
    void screenOrder();
};

void tst_output::primaryScreen()
{
    // Verify that the client has bound to the output global
    QCOMPOSITOR_TRY_COMPARE(output()->resourceMap().size(), 1);
    QTRY_VERIFY(QGuiApplication::primaryScreen());
    QScreen *screen = QGuiApplication::primaryScreen();
    QCOMPARE(screen->manufacturer(), "Make");
    QCOMPARE(screen->model(), "Model");
    QCOMPARE(screen->size(), QSize(1920, 1080));
    QCOMPARE(screen->refreshRate(), 60);
    QCOMPARE(qRound(screen->physicalDotsPerInch()), 96 / screen->devicePixelRatio());
    QCOMPARE(screen->devicePixelRatio(), 1);
    QCOMPARE(screen->logicalDotsPerInch(), 96);
}

void tst_output::secondaryHiDpiScreen()
{
    exec([=] {
        OutputData d;
        d.position = {1920, 0}; // in global compositor space (not pixels)
        d.mode.resolution = {800, 640};
        d.physicalSize = d.mode.physicalSizeForDpi(200);
        d.scale = 2;
        add<Output>(d);
    });

    // Verify that the client has bound to the output global
    QCOMPOSITOR_TRY_VERIFY(output(1) && output(1)->resourceMap().size() == 1);

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QScreen *screen = QGuiApplication::screens()[1];
    QCOMPARE(screen->refreshRate(), 60);
    QCOMPARE(screen->devicePixelRatio(), 2);
    QCOMPARE(screen->logicalDotsPerInch(), 96);

    // Dots currently means device pixels, not actual pixels (see QTBUG-62649)
    QCOMPARE(qRound(screen->physicalDotsPerInch() * screen->devicePixelRatio()), 200);

    // Size is in logical pixel coordinates
    QCOMPARE(screen->size(), QSize(800, 640) / 2);
    QCOMPARE(screen->geometry(), QRect(QPoint(1920, 0), QSize(400, 320)));
    QCOMPARE(screen->virtualGeometry(), QRect(QPoint(0, 0), QSize(1920 + 800 / 2, 1080)));

    exec([=] { remove(output(1)); });
}

// QTBUG-62044
void tst_output::addScreenWithGeometryChange()
{
    const QPoint initialPosition = exec([=] { return output(0)->m_data.position; });

    exec([=] {
        auto *oldOutput = output(0);
        auto *newOutput = add<Output>();
        newOutput->m_data.mode.resolution = {1280, 720};
        // Move the primary output to the right
        QPoint newPosition(newOutput->m_data.mode.resolution.width(), 0);
        Q_ASSERT(newPosition != initialPosition);
        oldOutput->m_data.position = newPosition;
        oldOutput->sendGeometry();
        oldOutput->sendDone();
    });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->geometry(), QRect(QPoint(1280, 0), QSize(1920, 1080)));

    // Remove the extra output and move the old one back
    exec([=] {
        remove(output(1));
        output()->m_data.position = initialPosition;
        output()->sendGeometry();
        output()->sendDone();
    });
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->geometry(), QRect(QPoint(0, 0), QSize(1920, 1080)));
}

void tst_output::windowScreens()
{
    QRasterWindow window;
    window.resize(400, 320);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QScreen *primaryScreen = QGuiApplication::screens().first();
    QCOMPARE(window.screen(), primaryScreen);

    exec([=] { add<Output>(); });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QScreen *secondaryScreen = QGuiApplication::screens().at(1);
    QVERIFY(secondaryScreen);

    window.setScreen(secondaryScreen);
    QCOMPARE(window.screen(), secondaryScreen);

    exec([=] {
        xdgToplevel()->surface()->sendEnter(output(0));
        xdgToplevel()->surface()->sendEnter(output(1));
    });

    QTRY_COMPARE(window.screen(), primaryScreen);

    exec([=] {
        xdgToplevel()->surface()->sendLeave(output(0));
    });
    QTRY_COMPARE(window.screen(), secondaryScreen);

    exec([=] {
        remove(output(1));
    });
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QCOMPARE(window.screen(), primaryScreen);
}

void tst_output::removePrimaryScreen()
{
    QRasterWindow window;
    window.resize(400, 320);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QScreen *primaryScreen = QGuiApplication::screens().first();
    QCOMPARE(window.screen(), primaryScreen);

    // Add a clone of the primary output
    exec([&] { add<Output>(output()->m_data); });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->virtualSiblings().size(), 2);
    QScreen *secondaryScreen = QGuiApplication::screens().at(1);
    QVERIFY(secondaryScreen);

    exec([&] { remove(output()); });
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);

    exec([&] {
        auto *surface = xdgToplevel()->surface();
        pointer()->sendEnter(surface, {32, 32});
        pointer()->sendButton(client(), BTN_LEFT, 1);
        pointer()->sendButton(client(), BTN_LEFT, 0);
    });

    // Wait to make sure mouse events dont't cause a crash now that the screen has changed
    xdgPingAndWaitForPong();
}

// QTBUG-72828
void tst_output::screenOrder()
{
    exec([=] {
        add<Output>()->m_data.model = "Screen 1";
        add<Output>()->m_data.model = "Screen 2";
    });

    QTRY_COMPARE(QGuiApplication::screens().size(), 3);
    const auto screens = QGuiApplication::screens();

    QCOMPARE(screens[1]->model(), "Screen 1");
    QCOMPARE(screens[2]->model(), "Screen 2");

    exec([=] {
        remove(output(2));
        remove(output(1));
    });
}

QCOMPOSITOR_TEST_MAIN(tst_output)
#include "tst_output.moc"
