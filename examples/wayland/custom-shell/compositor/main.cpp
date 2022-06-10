// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>

#include "exampleshell.h"

static void registerTypes()
{
    qmlRegisterType<ExampleShellQuickExtension>("io.qt.examples", 1, 0, "ExampleShell");
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    registerTypes();
    QQmlApplicationEngine appEngine(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}
