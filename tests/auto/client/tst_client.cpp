/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockcompositor.h"

#include <QtTest/QtTest>

static const QSize screenSize(1600, 1200);

class TestWindow : public QWindow
{
public:
    TestWindow()
        : focusInEventCount(0)
        , focusOutEventCount(0)
        , keyPressEventCount(0)
        , keyReleaseEventCount(0)
        , mousePressEventCount(0)
        , mouseReleaseEventCount(0)
    {
        setSurfaceType(QSurface::RasterSurface);
        setGeometry(0, 0, 32, 32);
        create();
    }

    void focusInEvent(QFocusEvent *)
    {
        ++focusInEventCount;
    }

    void focusOutEvent(QFocusEvent *)
    {
        ++focusOutEventCount;
    }

    void keyPressEvent(QKeyEvent *)
    {
        ++keyPressEventCount;
    }

    void keyReleaseEvent(QKeyEvent *)
    {
        ++keyReleaseEventCount;
    }

    void mousePressEvent(QMouseEvent *event)
    {
        ++mousePressEventCount;
        mousePressPos = event->pos();
    }

    void mouseReleaseEvent(QMouseEvent *)
    {
        ++mouseReleaseEventCount;
    }

    int focusInEventCount;
    int focusOutEventCount;
    int keyPressEventCount;
    int keyReleaseEventCount;
    int mousePressEventCount;
    int mouseReleaseEventCount;

    QPoint mousePressPos;
};

class tst_WaylandClient : public QObject
{
    Q_OBJECT
public:
    tst_WaylandClient(MockCompositor *c)
        : compositor(c)
    {
        QSocketNotifier *notifier = new QSocketNotifier(compositor->waylandFileDescriptor(), QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));
    }

public slots:
    void processWaylandEvents()
    {
        compositor->processWaylandEvents();
    }

private slots:
    void screen();
    void createDestroyWindow();
    void events();

private:
    MockCompositor *compositor;
};

void tst_WaylandClient::screen()
{
    QTRY_COMPARE(QGuiApplication::primaryScreen()->size(), screenSize);
}

void tst_WaylandClient::createDestroyWindow()
{
    TestWindow window;
    window.show();

    QTRY_VERIFY(compositor->surface());

    window.destroy();
    QTRY_VERIFY(!compositor->surface());
}

void tst_WaylandClient::events()
{
    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());

    QCOMPARE(window.focusInEventCount, 0);
    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(window.focusInEventCount, 1);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    QCOMPARE(window.focusOutEventCount, 0);
    compositor->setKeyboardFocus(QSharedPointer<MockSurface>(0));
    QTRY_COMPARE(window.focusOutEventCount, 1);
    QTRY_COMPARE(QGuiApplication::focusWindow(), static_cast<QWindow *>(0));

    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(window.focusInEventCount, 2);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    QPoint mousePressPos(16, 16);
    QCOMPARE(window.mousePressEventCount, 0);
    compositor->sendMousePress(surface, mousePressPos);
    QTRY_COMPARE(window.mousePressEventCount, 1);
    QTRY_COMPARE(window.mousePressPos, mousePressPos);

    QCOMPARE(window.mouseReleaseEventCount, 0);
    compositor->sendMouseRelease(surface);
    QTRY_COMPARE(window.mouseReleaseEventCount, 1);
}

int main(int argc, char **argv)
{
    setenv("XDG_RUNTIME_DIR", ".", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin

    MockCompositor compositor;
    compositor.setOutputGeometry(QRect(QPoint(), screenSize));

    QGuiApplication app(argc, argv);
    compositor.applicationInitialized();

    tst_WaylandClient tc(&compositor);
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_client.moc>
