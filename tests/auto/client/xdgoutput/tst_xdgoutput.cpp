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

#include "xdgoutputv1.h"
#include "mockcompositor.h"

#include <QtGui/QRasterWindow>
#include <QtGui/QOpenGLWindow>
#include <QtGui/QScreen>

using namespace MockCompositor;

class XdgOutputV1Compositor : public DefaultCompositor {
public:
    explicit XdgOutputV1Compositor()
    {
        exec([this] {
            int version = 3; // version 3 of of unstable-v1
            add<XdgOutputManagerV1>(version);
        });
    }
    XdgOutputV1 *xdgOutput(int i = 0) { return get<XdgOutputManagerV1>()->getXdgOutput(output(i)); }
};

class tst_xdgoutput : public QObject, private XdgOutputV1Compositor
{
    Q_OBJECT
private slots:
    void cleanup();
    void primaryScreen();
    void overrideGeometry();
    void changeGeometry();
};

void tst_xdgoutput::cleanup()
{
    QCOMPOSITOR_COMPARE(getAll<Output>().size(), 1); // Only the default output should be left
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage()));
}

void tst_xdgoutput::primaryScreen()
{
    // Verify that the client has bound to the global
    QCOMPOSITOR_TRY_COMPARE(get<XdgOutputManagerV1>()->resourceMap().size(), 1);
    exec([=] {
        auto *resource = xdgOutput()->resourceMap().value(client());
        QCOMPARE(resource->version(), 3);
        QCOMPARE(xdgOutput()->m_logicalGeometry.size(), QSize(1920, 1080));
    });
    auto *s = QGuiApplication::primaryScreen();
    QTRY_COMPARE(s->size(), QSize(1920, 1080));
    QTRY_COMPARE(s->geometry().topLeft(), QPoint(0, 0));
    QTRY_COMPARE(s->name(), QString("WL-1"));
}

void tst_xdgoutput::overrideGeometry()
{
    exec([=] {
        auto *output = add<Output>();
        auto *xdgOutput = get<XdgOutputManagerV1>()->getXdgOutput(output);
        xdgOutput->m_logicalGeometry = QRect(10, 20, 800, 1200);
    });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    auto *s = QGuiApplication::screens()[1];

    QTRY_COMPARE(s->size(), QSize(800, 1200));
    QTRY_COMPARE(s->geometry().topLeft(), QPoint(10, 20));

    exec([=] { remove(output(1)); });
}

void tst_xdgoutput::changeGeometry()
{
    auto *xdgOutput = exec([=] {
        auto *output = add<Output>();
        auto *xdgOutput = get<XdgOutputManagerV1>()->getXdgOutput(output);
        xdgOutput->m_logicalGeometry = QRect(10, 20, 800, 1200);
        return xdgOutput;
    });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    auto *screen = QGuiApplication::screens()[1];
    QTRY_COMPARE(screen->size(), QSize(800, 1200));

    exec([=] {
        xdgOutput->sendLogicalSize(QSize(1024, 768));
    });

    // Now we want to check that the client doesn't apply the size immediately, but waits for the
    // done event. If we TRY_COMPARE immediately, we risk that the client just hasn't handled the
    // logical_size request yet, so we add a screen and verify it on the client side just to give
    // the client a chance to mess up.
    exec([=] { add<Output>(); });
    QTRY_COMPARE(QGuiApplication::screens().size(), 3);
    exec([=] { remove(output(2)); });

    // The logical_size event should have been handled by now, but state should not have been applied yet.
    QTRY_COMPARE(screen->size(), QSize(800, 1200));

    exec([=] {
        xdgOutput->m_output->sendDone();
    });

    // Finally, the size should change
    QTRY_COMPARE(screen->size(), QSize(1024, 768));

    exec([=] { remove(output(1)); });
}

QCOMPOSITOR_TEST_MAIN(tst_xdgoutput)
#include "tst_xdgoutput.moc"
