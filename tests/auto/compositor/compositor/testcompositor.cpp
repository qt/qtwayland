// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testcompositor.h"
#include "testseat.h"
#include "testkeyboardgrabber.h"

#include <wayland-server-core.h>

TestCompositor::TestCompositor(bool createInputDev)
    : shell(new QWaylandWlShell(this))
    , m_createSeat(createInputDev)
{
    setSocketName("wayland-qt-test-0");
}

void TestCompositor::create()
{
    auto output = new QWaylandOutput(this, nullptr);
    setDefaultOutput(output);

    QWaylandCompositor::create();

    connect(this, &QWaylandCompositor::surfaceCreated, this, &TestCompositor::onSurfaceCreated);
    connect(this, &QWaylandCompositor::surfaceAboutToBeDestroyed, this, &TestCompositor::onSurfaceAboutToBeDestroyed);
}

void TestCompositor::flushClients()
{
    wl_display_flush_clients(display());
}

void TestCompositor::onSurfaceCreated(QWaylandSurface *surface)
{
    surfaces << surface;
}

void TestCompositor::onSurfaceAboutToBeDestroyed(QWaylandSurface *surface)
{
    surfaces.removeOne(surface);
}

QWaylandSeat *TestCompositor::createSeat()
{
    if (m_createSeat)
        return new TestSeat(this, QWaylandSeat::Pointer | QWaylandSeat::Keyboard);
    else
        return QWaylandCompositor::createSeat();
}

QWaylandKeyboard *TestCompositor::createKeyboardDevice(QWaylandSeat *seat) {
    return new TestKeyboardGrabber(seat);
}
