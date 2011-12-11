/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylanddisplay.h"

#include "qwaylandwindow.h"
#include "qwaylandscreen.h"
#include "qwaylandcursor.h"
#include "qwaylandinputdevice.h"
#include "qwaylandclipboard.h"
#include "qwaylanddatadevicemanager.h"

#ifdef QT_WAYLAND_GL_SUPPORT
#include "gl_integration/qwaylandglintegration.h"
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <QMutex>

// lock used to syncronize swap-buffers with the display-event-loop
QMutex g_waylandLock;

#include <QtCore/QDebug>

struct wl_surface *QWaylandDisplay::createSurface(void *handle)
{
    struct wl_surface * surface = wl_compositor_create_surface(mCompositor);
    wl_surface_set_user_data(surface, handle);
    return surface;
}

#ifdef QT_WAYLAND_GL_SUPPORT
QWaylandGLIntegration * QWaylandDisplay::eglIntegration()
{
    return mEglIntegration;
}
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
QWaylandWindowManagerIntegration *QWaylandDisplay::windowManagerIntegration()
{
    return mWindowManagerIntegration;
}
#endif

QWaylandInputDevice *QWaylandDisplay::lastKeyboardFocusInputDevice() const
{
    return mLastKeyboardFocusInputDevice;
}

void QWaylandDisplay::setLastKeyboardFocusInputDevice(QWaylandInputDevice *device)
{
    qDebug() << "setting last keyboard focus input device" << device;
    mLastKeyboardFocusInputDevice = device;
}

void QWaylandDisplay::shellHandleConfigure(void *data, struct wl_shell *shell,
                                           uint32_t time, uint32_t edges,
                                           struct wl_surface *surface,
                                           int32_t width, int32_t height)
{
    Q_UNUSED(data);
    Q_UNUSED(shell);
    Q_UNUSED(time);
    Q_UNUSED(edges);
    QWaylandWindow *ww = (QWaylandWindow *) wl_surface_get_user_data(surface);

    ww->configure(time, edges, 0, 0, width, height);
}

const struct wl_shell_listener QWaylandDisplay::shellListener = {
    QWaylandDisplay::shellHandleConfigure,
};

static QWaylandDisplay *display = 0;

QWaylandDisplay::QWaylandDisplay(void)
    : mDndSelectionHandler(0)
    , mLastKeyboardFocusInputDevice(0)
{
    display = this;
    qRegisterMetaType<uint32_t>("uint32_t");

    mDisplay = wl_display_connect(NULL);
    if (mDisplay == NULL) {
        qErrnoWarning(errno, "Failed to create display");
        qFatal("No wayland connection available.");
    }

    wl_display_add_global_listener(mDisplay, QWaylandDisplay::displayHandleGlobal, this);

    mFd = wl_display_get_fd(mDisplay, sourceUpdate, this);

#ifdef QTWAYLAND_EXPERIMENTAL_THREAD_SUPPORT
    mWritableNotificationFd = wl_display_get_write_notification_fd(mDisplay);
    QSocketNotifier *wn = new QSocketNotifier(mWritableNotificationFd, QSocketNotifier::Read, this);
    connect(wn, SIGNAL(activated(int)), this, SLOT(flushRequests()));
#else
    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(aboutToBlock()), this, SLOT(flushRequests()));
#endif

    mReadNotifier = new QSocketNotifier(mFd, QSocketNotifier::Read, this);
    connect(mReadNotifier, SIGNAL(activated(int)), this, SLOT(readEvents()));

#ifdef QT_WAYLAND_GL_SUPPORT
    mEglIntegration = QWaylandGLIntegration::createGLIntegration(this);
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    mWindowManagerIntegration = QWaylandWindowManagerIntegration::createIntegration(this);
#endif

    blockingReadEvents();

#ifdef QT_WAYLAND_GL_SUPPORT
    mEglIntegration->initialize();
#endif

    waitForScreens();
}

QWaylandDisplay::~QWaylandDisplay(void)
{
    close(mFd);
#ifdef QT_WAYLAND_GL_SUPPORT
    delete mEglIntegration;
#endif
    wl_display_destroy(mDisplay);
}

void QWaylandDisplay::createNewScreen(struct wl_output *output, QRect geometry)
{
    QWaylandScreen *waylandScreen = new QWaylandScreen(this,output,geometry);
    mScreens.append(waylandScreen);
}

void QWaylandDisplay::flushRequests()
{
    if (mSocketMask & WL_DISPLAY_WRITABLE)
        wl_display_iterate(mDisplay, WL_DISPLAY_WRITABLE);
}

void QWaylandDisplay::readEvents()
{
    if (g_waylandLock.tryLock())
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(mFd, &fds);

        fd_set nds;
        FD_ZERO(&nds);
        fd_set rs = fds;
        fd_set ws = nds;
        fd_set es = nds;
        timeval timeout;

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        int ret = ::select(mFd+1, &rs, &ws, &es, &timeout );

        if (ret <= 0) {
            g_waylandLock.unlock();
            return;
        }

        wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
        g_waylandLock.unlock();
    }
}

void QWaylandDisplay::blockingReadEvents()
{
    wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
}

int QWaylandDisplay::sourceUpdate(uint32_t mask, void *data)
{
    QWaylandDisplay *waylandDisplay = static_cast<QWaylandDisplay *>(data);
    waylandDisplay->mSocketMask = mask;

    return 0;
}

QWaylandScreen *QWaylandDisplay::screenForOutput(struct wl_output *output) const
{
    for (int i = 0; i < mScreens.size(); ++i) {
        QWaylandScreen *screen = static_cast<QWaylandScreen *>(mScreens.at(i));
        if (screen->output() == output)
            return screen;
    }
    return 0;
}

void QWaylandDisplay::outputHandleGeometry(void *data,
                                           wl_output *output,
                                           int32_t x, int32_t y,
                                           int32_t physicalWidth,
                                           int32_t physicalHeight,
                                           int subpixel,
                                           const char *make, const char *model)
{
    QWaylandDisplay *waylandDisplay = static_cast<QWaylandDisplay *>(data);
    QRect outputRect = QRect(x, y, physicalWidth, physicalHeight);
    waylandDisplay->createNewScreen(output,outputRect);
}

void QWaylandDisplay::mode(void *data,
             struct wl_output *wl_output,
             uint32_t flags,
             int width,
             int height,
             int refresh)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_output);
    Q_UNUSED(flags);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(refresh);
}

const struct wl_output_listener QWaylandDisplay::outputListener = {
    QWaylandDisplay::outputHandleGeometry,
    QWaylandDisplay::mode
};

void QWaylandDisplay::waitForScreens()
{
    flushRequests();
    while (mScreens.isEmpty())
        blockingReadEvents();
}

void QWaylandDisplay::displayHandleGlobal(struct wl_display *display,
                                          uint32_t id,
                                          const char *interface,
                                          uint32_t version,
                                          void *data)
{
    Q_UNUSED(display);
    QWaylandDisplay *that = static_cast<QWaylandDisplay *>(data);
    that->displayHandleGlobal(id, QByteArray(interface), version);
}

void QWaylandDisplay::displayHandleGlobal(uint32_t id,
                                          const QByteArray &interface,
                                          uint32_t version)
{
    Q_UNUSED(version);
    if (interface == "wl_output") {
        struct wl_output *output = static_cast<struct wl_output *>(wl_display_bind(mDisplay,id,&wl_output_interface));
        wl_output_add_listener(output, &outputListener, this);
    } else if (interface == "wl_compositor") {
        mCompositor = static_cast<struct wl_compositor *>(wl_display_bind(mDisplay, id,&wl_compositor_interface));
    } else if (interface == "wl_shm") {
        mShm = static_cast<struct wl_shm *>(wl_display_bind(mDisplay, id, &wl_shm_interface));
    } else if (interface == "wl_shell"){
        mShell = static_cast<struct wl_shell *>(wl_display_bind(mDisplay, id, &wl_shell_interface));
        wl_shell_add_listener(mShell, &shellListener, this);
    } else if (interface == "wl_input_device") {
        QWaylandInputDevice *inputDevice =
            new QWaylandInputDevice(this, id);
        mInputDevices.append(inputDevice);
    } else if (interface == "wl_data_device_manager") {
        mDndSelectionHandler = new QWaylandDataDeviceManager(this, id);
    }
}

uint32_t QWaylandDisplay::currentTimeMillisec()
{
    //### we throw away the time information
    struct timeval tv;
    int ret = gettimeofday(&tv, 0);
    if (ret == 0)
        return tv.tv_sec*1000 + tv.tv_usec/1000;
    return 0;
}

void QWaylandDisplay::force_roundtrip_sync_callback(void *data, struct wl_callback *wl_callback, uint32_t time)
{
    Q_UNUSED(time);

    int *round_trip = (int *)data;
    *round_trip = true;
    wl_callback_destroy(wl_callback);
}

const struct wl_callback_listener QWaylandDisplay::force_roundtrip_sync_callback_listener = {
    QWaylandDisplay::force_roundtrip_sync_callback
};

void QWaylandDisplay::forceRoundTrip()
{
    int round_trip = false;
    wl_callback *sync_callback = wl_display_sync(mDisplay);
    wl_callback_add_listener(sync_callback,&force_roundtrip_sync_callback_listener,&round_trip);
    flushRequests();
    while (!round_trip) {
        readEvents();
    }
}
