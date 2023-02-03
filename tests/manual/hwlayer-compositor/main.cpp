// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <QtGui/QGuiApplication>
#include <QQmlContext>

#include <QtQml/QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine appEngine(QUrl("qrc:///main.qml"));
    return app.exec();
}
