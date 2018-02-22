/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <QBackingStore>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QMimeData>
#include <QPixmap>
#include <QDrag>

#include <QtTest/QtTest>

static const QSize screenSize(1600, 1200);

class TestWindow : public QWindow
{
public:
    TestWindow()
    {
        setSurfaceType(QSurface::RasterSurface);
        setGeometry(0, 0, 32, 32);
        create();
    }
};

class tst_WaylandClientXdgShellV6 : public QObject
{
    Q_OBJECT
public:
    tst_WaylandClientXdgShellV6(MockCompositor *c)
        : m_compositor(c)
    {
        QSocketNotifier *notifier = new QSocketNotifier(m_compositor->waylandFileDescriptor(), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &tst_WaylandClientXdgShellV6::processWaylandEvents);
        // connect to the event dispatcher to make sure to flush out the outgoing message queue
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake, this, &tst_WaylandClientXdgShellV6::processWaylandEvents);
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, &tst_WaylandClientXdgShellV6::processWaylandEvents);
    }

public slots:
    void processWaylandEvents()
    {
        m_compositor->processWaylandEvents();
    }

    void cleanup()
    {
        // make sure the surfaces from the last test are properly cleaned up
        // and don't show up as false positives in the next test
        QTRY_VERIFY(!m_compositor->surface());
        QTRY_VERIFY(!m_compositor->xdgToplevelV6());
    }

private slots:
    void createDestroyWindow();
    void configure();

private:
    MockCompositor *m_compositor = nullptr;
};

void tst_WaylandClientXdgShellV6::createDestroyWindow()
{
    TestWindow window;
    window.show();

    QTRY_VERIFY(m_compositor->surface());

    window.destroy();
    QTRY_VERIFY(!m_compositor->surface());
}

void tst_WaylandClientXdgShellV6::configure()
{
    QSharedPointer<MockOutput> output;
    QTRY_VERIFY(output = m_compositor->output());

    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = m_compositor->surface());

    m_compositor->processWaylandEvents();
    QTRY_VERIFY(window.isVisible());
    QTRY_VERIFY(!window.isExposed()); //Window should not be exposed before the first configure event

    //TODO: according to xdg-shell protocol, a buffer should not be attached to a the surface
    //until it's configured. Ensure this in the test!

    QSharedPointer<MockXdgToplevelV6> toplevel;
    QTRY_VERIFY(toplevel = m_compositor->xdgToplevelV6());
    const QSize newSize(123, 456);
    m_compositor->sendXdgToplevelV6Configure(toplevel, newSize);
    QTRY_VERIFY(window.isExposed());
    QTRY_COMPARE(window.frameGeometry(), QRect(QPoint(), newSize));
}

int main(int argc, char **argv)
{
    setenv("XDG_RUNTIME_DIR", ".", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin
    setenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell-v6", 1);

    // wayland-egl hangs in the test setup when we try to initialize. Until it gets
    // figured out, avoid clientBufferIntegration() from being called in
    // QWaylandWindow::createDecorations().
    setenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1", 1);

    MockCompositor compositor;
    compositor.setOutputMode(screenSize);

    QGuiApplication app(argc, argv);
    compositor.applicationInitialized();

    tst_WaylandClientXdgShellV6 tc(&compositor);
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_xdgshellv6.moc>
