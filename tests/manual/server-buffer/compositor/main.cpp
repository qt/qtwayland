// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>

#include "sharebufferextension.h"

static void registerTypes()
{
    qmlRegisterType<ShareBufferExtensionQuickExtension>("io.qt.examples.sharebufferextension", 1, 0, "ShareBufferExtension");
}

int main(int argc, char *argv[])
{
    // Make sure there is a valid OpenGL context in the main thread
    qputenv("QSG_RENDER_LOOP", "basic");

    QGuiApplication app(argc, argv);
    registerTypes();
    QQmlApplicationEngine appEngine(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}
