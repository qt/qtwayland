// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>

#include "customextension.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine appEngine(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}
