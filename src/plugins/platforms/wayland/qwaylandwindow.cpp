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
#include "qwaylandextendedsurface.h"
#include "qwaylandsubsurface.h"
#include "qwaylanddecoration.h"

#include <QtGui/QWindow>

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

#include <QCoreApplication>
#include <QtGui/QWindowSystemInterface>

#include <QtCore/QDebug>

QWaylandWindow::QWaylandWindow(QWindow *window)
    : QPlatformWindow(window)
    , mDisplay(QWaylandScreen::waylandScreenFromWindow(window)->display())
    , mSurface(mDisplay->createSurface(this))
    , mShellSurface(0)
    , mExtendedWindow(0)
    , mSubSurfaceWindow(0)
    , mWindowDecoration(0)
    , mMouseEventsInContentArea(false)
    , mMousePressedInContentArea(Qt::NoButton)
    , mBuffer(0)
    , mWaitingForFrameSync(false)
    , mFrameCallback(0)
    , mSentInitialResize(false)
{
    static WId id = 1;
    mWindowId = id++;

    if (mDisplay->shell())
        mShellSurface = mDisplay->shell()->getShellSurface(this);
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
    }else if (window->transientParent()) {
        if (window->transientParent()) {
            mShellSurface->updateTransientParent(window->transientParent());
        } else {
            mShellSurface->setTopLevel();
        }
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

void QWaylandWindow::setWindowTitle(const QString &title)
{
    Q_UNUSED(title);
    if (mWindowDecoration && window()->visible()) {
        mWindowDecoration->paintDecoration();
    }
}

void QWaylandWindow::setGeometry(const QRect &rect)
{
    QPlatformWindow::setGeometry(rect);

    if (shellSurface() && window()->transientParent())
        shellSurface()->updateTransientParent(window()->transientParent());
}

void QWaylandWindow::setVisible(bool visible)
{

    if (visible) {
        if (mBuffer)
            wl_surface_attach(mSurface, mBuffer->buffer(), 0, 0);

        if (!mSentInitialResize) {
            QWindowSystemInterface::handleSynchronousGeometryChange(window(), geometry());
            mSentInitialResize = true;
        }

        QWindowSystemInterface::handleSynchronousExposeEvent(window(), QRect(QPoint(), geometry().size()));
    } else {
        QWindowSystemInterface::handleSynchronousExposeEvent(window(), QRect(QPoint(), geometry().size()));
        wl_surface_attach(mSurface, 0,0,0);
        damage(QRect(QPoint(0,0),geometry().size()));
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


void QWaylandWindow::configure(uint32_t edges, int32_t width, int32_t height)
{
    Q_UNUSED(edges);

    int widthWithoutMargins = qMax(width-(frameMargins().left() +frameMargins().right()),1);
    int heightWithoutMargins = qMax(height-(frameMargins().top()+frameMargins().bottom()),1);

    widthWithoutMargins = qMax(widthWithoutMargins, window()->minimumSize().width());
    heightWithoutMargins = qMax(heightWithoutMargins, window()->minimumSize().height());
    QRect geometry = QRect(0,0,
                           widthWithoutMargins, heightWithoutMargins);
    setGeometry(geometry);
    QWindowSystemInterface::handleGeometryChange(window(), geometry);
}

void QWaylandWindow::attach(QWaylandBuffer *buffer)
{
    mBuffer = buffer;

    if (window()->isVisible()) {
        wl_surface_attach(mSurface, mBuffer->buffer(),0,0);
    }
}

QWaylandBuffer *QWaylandWindow::attached() const
{
    return mBuffer;
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
    if (!mWaitingForFrameSync)
        return;
    mDisplay->flushRequests();
    while (mWaitingForFrameSync)
        mDisplay->blockingReadEvents();
}

QMargins QWaylandWindow::frameMargins() const
{
    if (mWindowDecoration)
        return mWindowDecoration->margins();
    return QPlatformWindow::frameMargins();
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

QWaylandDecoration *QWaylandWindow::decoration() const
{
    return mWindowDecoration;
}

void QWaylandWindow::setDecoration(QWaylandDecoration *decoration)
{
    mWindowDecoration = decoration;
    if (subSurfaceWindow()) {
        subSurfaceWindow()->adjustPositionOfChildren();
    }
}

void QWaylandWindow::handleMouse(QWaylandInputDevice *inputDevice, ulong timestamp, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    if (mWindowDecoration) {
        handleMouseEventWithDecoration(inputDevice, timestamp,local,global,b,mods);
        return;
    }

    QWindowSystemInterface::handleMouseEvent(window(),timestamp,local,global,b,mods);
}

void QWaylandWindow::handleMouseEnter()
{
    if (!mWindowDecoration) {
        QWindowSystemInterface::handleEnterEvent(window());
    }
}

void QWaylandWindow::handleMouseLeave()
{
    if (mWindowDecoration) {
        if (mMouseEventsInContentArea) {
            QWindowSystemInterface::handleLeaveEvent(window());
        }
        mWindowDecoration->restoreMouseCursor();
    } else {
        QWindowSystemInterface::handleLeaveEvent(window());
    }
}

void QWaylandWindow::handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice, ulong timestamp, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    if (mWindowDecoration->inMouseButtonPressedState()) {
        mWindowDecoration->handleMouse(inputDevice,local,global,b,mods);
        return;
    }

    QPointF localTranslated = local;
    QMargins marg = frameMargins();
    QRect windowRect(0 + marg.left(),
                     0 + marg.top(),
                     geometry().size().width() - marg.right(),
                     geometry().size().height() - marg.bottom());
    if (windowRect.contains(local.toPoint()) || mMousePressedInContentArea != Qt::NoButton) {
        localTranslated.setX(localTranslated.x() - marg.left());
        localTranslated.setY(localTranslated.y() - marg.top());
        if (!mMouseEventsInContentArea) {
            mWindowDecoration->restoreMouseCursor();
            QWindowSystemInterface::handleEnterEvent(window());
        }
        QWindowSystemInterface::handleMouseEvent(window(),timestamp,localTranslated,global,b,mods);
        mMouseEventsInContentArea = true;
        mMousePressedInContentArea = b;
    } else {
        if (mMouseEventsInContentArea) {
            QWindowSystemInterface::handleLeaveEvent(window());
            mMouseEventsInContentArea = false;
        }
        mWindowDecoration->handleMouse(inputDevice,local,global,b,mods);
    }
}
