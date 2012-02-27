/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockclient.h"
#include "testcompositor.h"

#include <QtTest/QtTest>

class tst_WaylandCompositor : public QObject
{
    Q_OBJECT

public:
    tst_WaylandCompositor() {
        setenv("XDG_RUNTIME_DIR", ".", 1);
    }

private slots:
    void singleClient();
    void multipleClients();
    void geometry();
    void mapSurface();
    void frameCallback();
};

void tst_WaylandCompositor::singleClient()
{
    TestCompositor compositor;

    MockClient client;

    wl_surface *sa = client.createSurface();
    QTRY_COMPARE(compositor.surfaces.size(), 1);

    wl_surface *sb = client.createSurface();
    QTRY_COMPARE(compositor.surfaces.size(), 2);

    wl_surface_destroy(sa);
    QTRY_COMPARE(compositor.surfaces.size(), 1);

    wl_surface_destroy(sb);
    QTRY_COMPARE(compositor.surfaces.size(), 0);
}

void tst_WaylandCompositor::multipleClients()
{
    TestCompositor compositor;

    MockClient a;
    MockClient b;
    MockClient c;

    wl_surface *sa = a.createSurface();
    wl_surface *sb = b.createSurface();
    wl_surface *sc = c.createSurface();

    QTRY_COMPARE(compositor.surfaces.size(), 3);

    wl_surface_destroy(sa);
    wl_surface_destroy(sb);
    wl_surface_destroy(sc);

    QTRY_COMPARE(compositor.surfaces.size(), 0);
}

void tst_WaylandCompositor::geometry()
{
    TestCompositor compositor;

    QRect geometry(0, 0, 4096, 3072);
    compositor.setOutputGeometry(geometry);

    MockClient client;

    QTRY_COMPARE(client.geometry, geometry);
}

void tst_WaylandCompositor::mapSurface()
{
    TestCompositor compositor;

    MockClient client;

    wl_surface *surface = client.createSurface();
    QTRY_COMPARE(compositor.surfaces.size(), 1);

    WaylandSurface *waylandSurface = compositor.surfaces.at(0);

    QSignalSpy mappedSpy(waylandSurface, SIGNAL(mapped()));

    QCOMPARE(waylandSurface->size(), QSize());
    QCOMPARE(waylandSurface->type(), WaylandSurface::Invalid);

    QSize size(256, 256);
    ShmBuffer buffer(size, client.shm);

    wl_surface_attach(surface, buffer.handle, 0, 0);
    wl_surface_damage(surface, 0, 0, size.width(), size.height());

    QTRY_COMPARE(waylandSurface->size(), size);
    QTRY_COMPARE(waylandSurface->type(), WaylandSurface::Shm);
    QTRY_COMPARE(mappedSpy.count(), 1);

    wl_surface_destroy(surface);
}

static void frameCallbackFunc(void *data, wl_callback *callback, uint32_t)
{
    ++*static_cast<int *>(data);
    wl_callback_destroy(callback);
}

static void registerFrameCallback(wl_surface *surface, int *counter)
{
    static const wl_callback_listener frameCallbackListener = {
        frameCallbackFunc
    };

    wl_callback_add_listener(wl_surface_frame(surface), &frameCallbackListener, counter);
}

void tst_WaylandCompositor::frameCallback()
{
    TestCompositor compositor;

    MockClient client;

    QSize size(8, 8);
    ShmBuffer buffer(size, client.shm);

    wl_surface *surface = client.createSurface();
    wl_surface_attach(surface, buffer.handle, 0, 0);

    int frameCounter = 0;

    QTRY_COMPARE(compositor.surfaces.size(), 1);
    WaylandSurface *waylandSurface = compositor.surfaces.at(0);
    QSignalSpy damagedSpy(waylandSurface, SIGNAL(damaged(const QRect &)));

    for (int i = 0; i < 10; ++i) {
        registerFrameCallback(surface, &frameCounter);
        wl_surface_damage(surface, 0, 0, size.width(), size.height());

        QTRY_COMPARE(waylandSurface->type(), WaylandSurface::Shm);
        QTRY_COMPARE(damagedSpy.count(), i + 1);

        QCOMPARE(waylandSurface->image(), buffer.image);
        waylandSurface->frameFinished();

        QTRY_COMPARE(frameCounter, i + 1);
    }

    wl_surface_destroy(surface);
}

#include <tst_compositor.moc>
QTEST_MAIN(tst_WaylandCompositor);
