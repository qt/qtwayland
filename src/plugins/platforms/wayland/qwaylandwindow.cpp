/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the config.tests of the Qt Toolkit.
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

#include "qwaylandwindow.h"

#include "qwaylandbuffer.h"
#include "qwaylanddisplay.h"
#include "qwaylandinputdevice.h"
#include "qwaylandscreen.h"
#include "qwaylandshell.h"
#include "qwaylandshellsurface.h"

#include <QtGui/QWindow>

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

#include "qwaylandextendedsurface.h"
#include "qwaylandsubsurface.h"

#include <QCoreApplication>
#include <QtGui/QWindowSystemInterface>

QWaylandWindow::QWaylandWindow(QWindow *window)
    : QPlatformWindow(window)
    , mDisplay(QWaylandScreen::waylandScreenFromWindow(window)->display())
    , mSurface(mDisplay->createSurface(this))
    , mShellSurface(mDisplay->shell()->createShellSurface(this))
    , mExtendedWindow(0)
    , mSubSurfaceWindow(0)
    , mBuffer(0)
    , mWaitingForFrameSync(false)
    , mFrameCallback(0)
{
    static WId id = 1;
    mWindowId = id++;

    if (mDisplay->windowExtension())
        mExtendedWindow = mDisplay->windowExtension()->getExtendedWindow(this);
    if (mDisplay->subSurfaceExtension())
        mSubSurfaceWindow = mDisplay->subSurfaceExtension()->getSubSurfaceAwareWindow(this);

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    mDisplay->windowManagerIntegration()->mapClientToProcess(qApp->applicationPid());
    mDisplay->windowManagerIntegration()->authenticateWithToken();
#endif

    if (parent() && mSubSurfaceWindow) {
        mSubSurfaceWindow->setParent(static_cast<const QWaylandWindow *>(parent()));
    } else {
        wl_shell_surface_set_toplevel(mShellSurface->handle());
    }
}

QWaylandWindow::~QWaylandWindow()
{
    if (mSurface) {
        delete mShellSurface;
        delete mExtendedWindow;
        wl_surface_destroy(mSurface);
    }

    QList<QWaylandInputDevice *> inputDevices = mDisplay->inputDevices();
    for (int i = 0; i < inputDevices.size(); ++i)
        inputDevices.at(i)->handleWindowDestroyed(this);
}

WId QWaylandWindow::winId() const
{
    return mWindowId;
}

void QWaylandWindow::setParent(const QPlatformWindow *parent)
{
    const QWaylandWindow *parentWaylandWindow = static_cast<const QWaylandWindow *>(parent);
    if (subSurfaceWindow()) {
        subSurfaceWindow()->setParent(parentWaylandWindow);
    }
}

void QWaylandWindow::setVisible(bool visible)
{

    if (visible) {
        if (mBuffer) {
            wl_surface_attach(mSurface, mBuffer->buffer(),0,0);
            QWindowSystemInterface::handleSynchronousExposeEvent(window(), QRect(QPoint(), geometry().size()));
        }
    } else {
        wl_surface_attach(mSurface, 0,0,0);
    }
}


bool QWaylandWindow::isExposed() const
{
    if (!window()->isVisible())
        return false;
    if (mExtendedWindow)
        return mExtendedWindow->isExposed();
    return true;
}


void QWaylandWindow::configure(uint32_t time, uint32_t edges,
                               int32_t x, int32_t y,
                               int32_t width, int32_t height)
{
    Q_UNUSED(time);
    Q_UNUSED(edges);

    QRect geometry = QRect(x, y, width, height);
    setGeometry(geometry);

    QWindowSystemInterface::handleGeometryChange(window(), geometry);
}

void QWaylandWindow::attach(QWaylandBuffer *buffer)
{
    mBuffer = buffer;

    if (window()->isVisible()) {
        wl_surface_attach(mSurface, mBuffer->buffer(),0,0);
        if (buffer)
            QWindowSystemInterface::handleSynchronousExposeEvent(window(), QRect(QPoint(), geometry().size()));
    }
}

void QWaylandWindow::damage(const QRect &rect)
{
    //We have to do sync stuff before calling damage, or we might
    //get a frame callback before we get the timestamp
    if (!mWaitingForFrameSync) {
        mFrameCallback = wl_surface_frame(mSurface);
        wl_callback_add_listener(mFrameCallback,&QWaylandWindow::callbackListener,this);
        mWaitingForFrameSync = true;
    }

    wl_surface_damage(mSurface,
                      rect.x(), rect.y(), rect.width(), rect.height());
}

const wl_callback_listener QWaylandWindow::callbackListener = {
    QWaylandWindow::frameCallback
};

void QWaylandWindow::frameCallback(void *data, struct wl_callback *wl_callback, uint32_t time)
{
    Q_UNUSED(time);
    Q_UNUSED(wl_callback);
    QWaylandWindow *self = static_cast<QWaylandWindow*>(data);
    self->mWaitingForFrameSync = false;
    if (self->mFrameCallback) {
        wl_callback_destroy(self->mFrameCallback);
        self->mFrameCallback = 0;
    }
}

void QWaylandWindow::waitForFrameSync()
{
    mDisplay->flushRequests();
    while (mWaitingForFrameSync)
        mDisplay->blockingReadEvents();
}

QWaylandShellSurface *QWaylandWindow::shellSurface() const
{
    return mShellSurface;
}

QWaylandExtendedSurface *QWaylandWindow::extendedWindow() const
{
    return mExtendedWindow;
}

QWaylandSubSurface *QWaylandWindow::subSurfaceWindow() const
{
    return mSubSurfaceWindow;
}

void QWaylandWindow::handleContentOrientationChange(Qt::ScreenOrientation orientation)
{
    if (mExtendedWindow)
        mExtendedWindow->setContentOrientation(orientation);
}

Qt::ScreenOrientation QWaylandWindow::requestWindowOrientation(Qt::ScreenOrientation orientation)
{
    if (mExtendedWindow) {
        mExtendedWindow->setWindowOrientation(orientation);
        return orientation;
    }

    return Qt::PrimaryOrientation;
}

Qt::WindowState QWaylandWindow::setWindowState(Qt::WindowState state)
{
    if (state == Qt::WindowFullScreen || state == Qt::WindowMaximized) {
        QScreen *screen = window()->screen();

        QRect geometry = screen->mapBetween(window()->windowOrientation(), screen->primaryOrientation(), screen->geometry());
        setGeometry(geometry);

        QWindowSystemInterface::handleGeometryChange(window(), geometry);

        return state;
    }

    return Qt::WindowNoState;
}

Qt::WindowFlags QWaylandWindow::setWindowFlags(Qt::WindowFlags flags)
{
    return mExtendedWindow->setWindowFlags(flags);
}
