// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    // ShareOpenGLContexts is needed for using the threaded renderer
    // on Nvidia EGLStreams
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine appEngine(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}
