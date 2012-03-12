/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
#include "qwaylandshell.h"

#ifdef QT_WAYLAND_GL_SUPPORT
#include "gl_integration/qwaylandglintegration.h"
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

#include "qwaylandextendedoutput.h"
#include "qwaylandextendedsurface.h"
#include "qwaylandsubsurface.h"
#include "qwaylandtouch.h"

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

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

static QWaylandDisplay *display = 0;

static int dummyUpdate(uint32_t, void *)
{
    return 0;
}

QWaylandDisplay::QWaylandDisplay(void)
    : mLastKeyboardFocusInputDevice(0)
    , mDndSelectionHandler(0)
    , mWindowExtension(0)
    , mSubSurfaceExtension(0)
    , mOutputExtension(0)
    , mTouchExtension(0)
{
    display = this;
    qRegisterMetaType<uint32_t>("uint32_t");

    mDisplay = wl_display_connect(NULL);
    if (mDisplay == NULL) {
        qErrnoWarning(errno, "Failed to create display");
        qFatal("No wayland connection available.");
    }

    wl_display_add_global_listener(mDisplay, QWaylandDisplay::displayHandleGlobal, this);

    mFd = wl_display_get_fd(mDisplay, dummyUpdate, 0);

#ifdef WAYLAND_CLIENT_THREAD_AFFINITY
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
    wl_display_flush(mDisplay);
}

void QWaylandDisplay::readEvents()
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
        return;
    }

    wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
}

void QWaylandDisplay::blockingReadEvents()
{
    wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
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
    Q_UNUSED(subpixel);
    Q_UNUSED(make);
    Q_UNUSED(model);
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
        mShell = new QWaylandShell(this,id,version);
    } else if (interface == "wl_input_device") {
        QWaylandInputDevice *inputDevice =
            new QWaylandInputDevice(this, id);
        mInputDevices.append(inputDevice);
    } else if (interface == "wl_data_device_manager") {
        mDndSelectionHandler = new QWaylandDataDeviceManager(this, id);
    } else if (interface == "wl_output_extension") {
        mOutputExtension = new QWaylandOutputExtension(this,id);
    } else if (interface == "wl_surface_extension") {
        mWindowExtension = new QWaylandSurfaceExtension(this,id);
    } else if (interface == "wl_sub_surface_extension") {
        mSubSurfaceExtension = new QWaylandSubSurfaceExtension(this,id);
    } else if (interface == "wl_touch_extension") {
        mTouchExtension = new QWaylandTouchExtension(this, id);
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

void QWaylandDisplay::forceRoundTrip()
{
    wl_display_roundtrip(mDisplay);
}

