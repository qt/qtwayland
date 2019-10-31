/****************************************************************************
**
** Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QWindow>

#include <QtTest/QtTest>

static const QSize screenSize(1600, 1200);

class TestWindow : public QWindow
{
public:
    TestWindow()
    {
        setSurfaceType(QSurface::RasterSurface);
        setGeometry(0, 0, 800, 600);
        create();
    }
};

class tst_WaylandClientFullScreenShellV1 : public QObject
{
    Q_OBJECT
public:
    tst_WaylandClientFullScreenShellV1(MockCompositor *c)
        : m_compositor(c)
    {
        QSocketNotifier *notifier = new QSocketNotifier(m_compositor->waylandFileDescriptor(), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &tst_WaylandClientFullScreenShellV1::processWaylandEvents);
        // connect to the event dispatcher to make sure to flush out the outgoing message queue
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake, this, &tst_WaylandClientFullScreenShellV1::processWaylandEvents);
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, &tst_WaylandClientFullScreenShellV1::processWaylandEvents);
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
        QTRY_VERIFY(!m_compositor->fullScreenShellV1Surface());
    }

private slots:
    void createDestroyWindow();

private:
    MockCompositor *m_compositor = nullptr;
};

void tst_WaylandClientFullScreenShellV1::createDestroyWindow()
{
    TestWindow window;
    window.show();

    QTRY_VERIFY(m_compositor->fullScreenShellV1Surface());

    window.destroy();
    QTRY_VERIFY(!m_compositor->fullScreenShellV1Surface());
}

int main(int argc, char **argv)
{
    QTemporaryDir tmpRuntimeDir;
    setenv("XDG_RUNTIME_DIR", tmpRuntimeDir.path().toLocal8Bit(), 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin
    setenv("QT_WAYLAND_SHELL_INTEGRATION", "fullscreen-shell-v1", 1);
    setenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1", 1); // window decorations don't make much sense here

    MockCompositor compositor;
    compositor.setOutputMode(screenSize);

    QGuiApplication app(argc, argv);
    compositor.applicationInitialized();

    tst_WaylandClientFullScreenShellV1 tc(&compositor);
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_fullscreenshellv1.moc>
