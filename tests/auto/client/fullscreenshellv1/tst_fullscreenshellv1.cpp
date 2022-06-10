// Copyright (C) 2021 David Edmundson <davidedmundson@kde.org>
// Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mockcompositor.h"

#include <QRasterWindow>

#include <QtTest/QtTest>

using namespace MockCompositor;

class tst_WaylandClientFullScreenShellV1 : public QObject, private DefaultCompositor
{
    Q_OBJECT

private slots:
    void createDestroyWindow();
};

void tst_WaylandClientFullScreenShellV1::createDestroyWindow()
{
    QRasterWindow window;
    window.resize(800, 600);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(fullScreenShellV1()->surfaces().count() == 1);
    QCOMPOSITOR_VERIFY(surface(0));

    window.destroy();
    QCOMPOSITOR_TRY_VERIFY(!surface(0));
}

int main(int argc, char **argv)
{
    QTemporaryDir tmpRuntimeDir;
    setenv("XDG_RUNTIME_DIR", tmpRuntimeDir.path().toLocal8Bit(), 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin
    setenv("QT_WAYLAND_SHELL_INTEGRATION", "fullscreen-shell-v1", 1);
    setenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1", 1); // window decorations don't make much sense here

    tst_WaylandClientFullScreenShellV1 tc;
    QGuiApplication app(argc, argv);
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_fullscreenshellv1.moc>
