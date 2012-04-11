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

#include "wayland-client.h"

#include <QObject>
#include <QImage>
#include <QRect>

class ShmBuffer
{
public:
    ShmBuffer(const QSize &size, wl_shm *shm);
    ~ShmBuffer();

    struct wl_buffer *handle;
    struct wl_shm_pool *shm_pool;
    QImage image;
};

class MockClient : public QObject
{
    Q_OBJECT

public:
    MockClient();
    ~MockClient();

    wl_surface *createSurface();

    wl_display *display;
    wl_compositor *compositor;
    wl_output *output;
    wl_shm *shm;

    QRect geometry;

    int fd;

private slots:
    void readEvents();
    void flushDisplay();

private:
    static MockClient *resolve(void *data) { return static_cast<MockClient *>(data); }

    static void handleGlobal(wl_display *display, uint32_t id, const char *interface, uint32_t, void *data);
    static int sourceUpdate(uint32_t mask, void *data);

    static void outputGeometryEvent(void *data,
                                    wl_output *output,
                                    int32_t x, int32_t y,
                                    int32_t width, int32_t height,
                                    int subpixel,
                                    const char *make,
                                    const char *model);

    static void outputModeEvent(void *data,
                                wl_output *wl_output,
                                uint32_t flags,
                                int width,
                                int height,
                                int refresh);

    void handleGlobal(uint32_t id, const QByteArray &interface);

    static const wl_output_listener outputListener;
};

