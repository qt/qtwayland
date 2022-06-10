// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QStandardPaths>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QDir>
#include <QTimer>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine appEngine;

    appEngine.load(QUrl("qrc:///main.qml"));

    return app.exec();
}
