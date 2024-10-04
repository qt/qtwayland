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

#include <QRasterWindow>

#include <QtTest/QtTest>

using namespace MockCompositor;

class tst_WaylandClientIviApplication : public QObject, private DefaultCompositor
{
    Q_OBJECT

private slots:
    void createDestroyWindow();
    void configure();
    void uniqueIviIds();
};

void tst_WaylandClientIviApplication::createDestroyWindow()
{
    QRasterWindow window;
    window.resize(32, 32);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(surface());
    QCOMPOSITOR_TRY_VERIFY(iviSurface());

    window.destroy();
    QCOMPOSITOR_TRY_VERIFY(!surface());
    QCOMPOSITOR_TRY_VERIFY(!iviSurface());
}

void tst_WaylandClientIviApplication::configure()
{
    QRasterWindow window;
    window.resize(32, 32);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(iviSurface());

    // Unconfigured ivi surfaces decide their own size
    QTRY_COMPARE(window.frameGeometry(), QRect(QPoint(), QSize(32, 32)));

    exec([&] {
        iviSurface()->send_configure(123, 456);
    });
    QTRY_COMPARE(window.frameGeometry(), QRect(QPoint(), QSize(123, 456)));
    window.destroy();
    QCOMPOSITOR_TRY_VERIFY(!iviSurface());
}

void tst_WaylandClientIviApplication::uniqueIviIds()
{
    QRasterWindow windowA, windowB;
    windowA.resize(32, 32);
    windowA.show();
    windowB.resize(32, 32);
    windowB.show();

    QCOMPOSITOR_TRY_VERIFY(iviSurface(0));
    QCOMPOSITOR_TRY_VERIFY(iviSurface(1));
       exec([&] {
            QVERIFY(iviSurface(0)->m_iviId != iviSurface(1)->m_iviId);
    });
}

int main(int argc, char **argv)
{
    QTemporaryDir tmpRuntimeDir;
    setenv("XDG_RUNTIME_DIR", tmpRuntimeDir.path().toLocal8Bit(), 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin
    setenv("QT_WAYLAND_SHELL_INTEGRATION", "ivi-shell", 1);
    setenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1", 1); // window decorations don't make much sense on ivi-application

    tst_WaylandClientIviApplication tc;
    QGuiApplication app(argc, argv);
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_iviapplication.moc>
