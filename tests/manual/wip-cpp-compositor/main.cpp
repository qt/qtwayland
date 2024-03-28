// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include "window.h"
#include "compositor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Compositor compositor;
    Window window(&compositor);
    compositor.setWindow(&window);

    window.resize(800, 600);
    window.show();

    return app.exec();
}
