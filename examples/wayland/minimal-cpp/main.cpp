// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include "window.h"
#include "compositor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Window window;
    window.resize(800,600);
    Compositor compositor(&window);
    window.show();

    return app.exec();
}
