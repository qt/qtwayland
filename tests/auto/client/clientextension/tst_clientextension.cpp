/****************************************************************************
**
** Copyright (C) 2021 David Redondo <qt@david-redondo.de>
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

#include <QSignalSpy>
#include <QtGui/private/qguiapplication_p.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/qwaylandclientextension.h>
#include <qwayland-server-test.h>
#include <qwayland-test.h>
#include "mockcompositor.h"
#include "coreprotocol.h"

using namespace MockCompositor;

class TestExtension : public QWaylandClientExtensionTemplate<TestExtension>,
                      public QtWayland::test_interface
{
public:
    TestExtension() : QWaylandClientExtensionTemplate<TestExtension>(1) { }
    ~TestExtension()
    {
        if (object()) {
            release();
        }
    }
    void initialize() { QWaylandClientExtension::initialize(); }
};

class TestGlobal : public Global, public QtWaylandServer::test_interface
{
    Q_OBJECT
public:
    explicit TestGlobal(CoreCompositor *compositor)
        : QtWaylandServer::test_interface(compositor->m_display, 1)
    {
    }
};

class tst_clientextension : public QObject, private CoreCompositor
{
    Q_OBJECT
private:
    QtWaylandClient::QWaylandDisplay *display()
    {
        return static_cast<QtWaylandClient::QWaylandIntegration *>(
                       QGuiApplicationPrivate::platformIntegration())
                ->display();
    }
private slots:
    void cleanup()
    {
        display()->flushRequests();
        dispatch();
        exec([this] { removeAll<TestGlobal>(); });
        QTRY_COMPARE(display()->globals().size(), 0);
    }
    void createWithoutGlobal();
    void createWithGlobalAutomatic();
    void createWithGlobalManual();
    void globalBecomesAvailable();
    void globalRemoved();
};

void tst_clientextension::createWithoutGlobal()
{
    TestExtension extension;
    QSignalSpy spy(&extension, &QWaylandClientExtension::activeChanged);
    QVERIFY(spy.isValid());
    QVERIFY(!extension.isActive());
    QCOMPARE(spy.count(), 0);
    extension.initialize();
    QVERIFY(!extension.isActive());
    QCOMPARE(spy.count(), 0);
}

void tst_clientextension::createWithGlobalAutomatic()
{
    exec([this] { add<TestGlobal>(); });
    QTRY_COMPARE(display()->globals().size(), 1);
    TestExtension extension;
    QSignalSpy spy(&extension, &QWaylandClientExtension::activeChanged);
    QVERIFY(spy.isValid());
    QTRY_VERIFY(extension.isActive());
    QCOMPARE(spy.count(), 1);
}

void tst_clientextension::createWithGlobalManual()
{
    exec([this] { add<TestGlobal>(); });
    QTRY_COMPARE(display()->globals().size(), 1);
    // Wait for the display to have the global
    TestExtension extension;
    QSignalSpy spy(&extension, &QWaylandClientExtension::activeChanged);
    QVERIFY(spy.isValid());
    extension.initialize();
    QVERIFY(extension.isActive());
    QCOMPARE(spy.count(), 1);
}

void tst_clientextension::globalBecomesAvailable()
{
    TestExtension extension;
    QSignalSpy spy(&extension, &QWaylandClientExtension::activeChanged);
    QVERIFY(spy.isValid());
    exec([this] { add<TestGlobal>(); });
    QTRY_VERIFY(extension.isActive());
    QCOMPARE(spy.count(), 1);
}

void tst_clientextension::globalRemoved()
{
    exec([this] { add<TestGlobal>(); });
    TestExtension extension;
    QTRY_VERIFY(extension.isActive());
    QSignalSpy spy(&extension, &QWaylandClientExtension::activeChanged);
    QVERIFY(spy.isValid());
    QCOMPOSITOR_TRY_COMPARE(get<TestGlobal>()->resourceMap().size(), 1);

    exec([this] { removeAll<TestGlobal>(); });
    QTRY_VERIFY(!extension.isActive());
    QCOMPARE(spy.count(), 1);
}

QCOMPOSITOR_TEST_MAIN(tst_clientextension)
#include "tst_clientextension.moc"
