/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandwindow_p.h"

#include "qwaylandbuffer_p.h"
#include "qwaylanddisplay_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandshellsurface_p.h"
#include "qwaylandwlshellsurface_p.h"
#include "qwaylandxdgsurface_p.h"
#include "qwaylandsubsurface_p.h"
#include "qwaylandabstractdecoration_p.h"
#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandnativeinterface_p.h"
#include "qwaylanddecorationfactory_p.h"
#include "qwaylandshmbackingstore_p.h"

#if QT_CONFIG(wayland_datadevice)
#include "qwaylanddatadevice_p.h"
#endif


#include <QtCore/QFileInfo>
#include <QtCore/QPointer>
#include <QtCore/QRegularExpression>
#include <QtGui/QWindow>

#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QtGui/private/qwindow_p.h>

#include <QtCore/QDebug>

#include <wayland-client-core.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_LOGGING_CATEGORY(lcWaylandBackingstore, "qt.qpa.wayland.backingstore")

QWaylandWindow *QWaylandWindow::mMouseGrab = nullptr;

QWaylandWindow::QWaylandWindow(QWindow *window)
    : QPlatformWindow(window)
    , mDisplay(waylandScreen()->display())
    , mResizeAfterSwap(qEnvironmentVariableIsSet("QT_WAYLAND_RESIZE_AFTER_SWAP"))
{
    static WId id = 1;
    mWindowId = id++;
    connect(qApp, &QGuiApplication::screenRemoved, this, &QWaylandWindow::handleScreenRemoved);
    initializeWlSurface();
}

QWaylandWindow::~QWaylandWindow()
{
    mDisplay->handleWindowDestroyed(this);

    delete mWindowDecoration;

    if (isInitialized())
        reset(false);

    QList<QWaylandInputDevice *> inputDevices = mDisplay->inputDevices();
    for (int i = 0; i < inputDevices.size(); ++i)
        inputDevices.at(i)->handleWindowDestroyed(this);

    const QWindow *parent = window();
    foreach (QWindow *w, QGuiApplication::topLevelWindows()) {
        if (w->transientParent() == parent)
            QWindowSystemInterface::handleCloseEvent(w);
    }

    if (mMouseGrab == this) {
        mMouseGrab = nullptr;
    }
}

void QWaylandWindow::ensureSize()
{
    if (mBackingStore)
        mBackingStore->ensureSize();
}

void QWaylandWindow::initWindow()
{
    if (window()->type() == Qt::Desktop)
        return;

    if (!isInitialized()) {
        initializeWlSurface();
        QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceCreated);
        QGuiApplication::sendEvent(window(), &e);
    }

    if (shouldCreateSubSurface()) {
        Q_ASSERT(!mSubSurfaceWindow);

        QWaylandWindow *p = static_cast<QWaylandWindow *>(QPlatformWindow::parent());
        if (::wl_subsurface *ss = mDisplay->createSubSurface(this, p)) {
            mSubSurfaceWindow = new QWaylandSubSurface(this, p, ss);
        }
    } else if (shouldCreateShellSurface()) {
        Q_ASSERT(!mShellSurface);

        mShellSurface = mDisplay->createShellSurface(this);
        if (mShellSurface) {
            mShellSurface->setType(window()->type(), transientParent());

            // Set initial surface title
            setWindowTitle(window()->title());

            // The appId is the desktop entry identifier that should follow the
            // reverse DNS convention (see http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s02.html).
            // According to xdg-shell the appId is only the name, without
            // the .desktop suffix.
            //
            // If the application specifies the desktop file name use that
            // removing the ".desktop" suffix, otherwise fall back to the
            // executable name and prepend the reversed organization domain
            // when available.
            if (!QGuiApplication::desktopFileName().isEmpty()) {
                QString name = QGuiApplication::desktopFileName();
                if (name.endsWith(QLatin1String(".desktop")))
                    name.chop(8);
                mShellSurface->setAppId(name);
            } else {
                QFileInfo fi = QCoreApplication::instance()->applicationFilePath();
                QStringList domainName =
                        QCoreApplication::instance()->organizationDomain().split(QLatin1Char('.'),
                                                                                 QString::SkipEmptyParts);

                if (domainName.isEmpty()) {
                    mShellSurface->setAppId(fi.baseName());
                } else {
                    QString appId;
                    for (int i = 0; i < domainName.count(); ++i)
                        appId.prepend(QLatin1Char('.')).prepend(domainName.at(i));
                    appId.append(fi.baseName());
                    mShellSurface->setAppId(appId);
                }
            }
            // the user may have already set some window properties, so make sure to send them out
            for (auto it = m_properties.cbegin(); it != m_properties.cend(); ++it)
                mShellSurface->sendProperty(it.key(), it.value());
        } else {
            qWarning("Could not create a shell surface object.");
        }
    }

    mScale = waylandScreen()->scale();

    // Enable high-dpi rendering. Scale() returns the screen scale factor and will
    // typically be integer 1 (normal-dpi) or 2 (high-dpi). Call set_buffer_scale()
    // to inform the compositor that high-resolution buffers will be provided.
    if (mDisplay->compositorVersion() >= 3)
        set_buffer_scale(scale());

    if (QScreen *s = window()->screen())
        setOrientationMask(s->orientationUpdateMask());
    setWindowFlags(window()->flags());
    if (window()->geometry().isEmpty())
        setGeometry_helper(QRect(QPoint(), QSize(500,500)));
    else
        setGeometry_helper(window()->geometry());
    setMask(window()->mask());
    // setWindowStateInternal is a no-op if the argument is equal to mState,
    // but since we're creating the shellsurface only now we reset mState to
    // make sure the state gets sent out to the compositor
    mState = Qt::WindowNoState;
    setWindowStateInternal(window()->windowStates());
    handleContentOrientationChange(window()->contentOrientation());
    mFlags = window()->flags();
}

void QWaylandWindow::initializeWlSurface()
{
    init(mDisplay->createSurface(static_cast<QtWayland::wl_surface *>(this)));
}

bool QWaylandWindow::shouldCreateShellSurface() const
{
    if (shouldCreateSubSurface())
        return false;

    if (window()->inherits("QShapedPixmapWindow"))
        return false;

    if (qEnvironmentVariableIsSet("QT_WAYLAND_USE_BYPASSWINDOWMANAGERHINT"))
        return !(window()->flags() & Qt::BypassWindowManagerHint);

    return true;
}

bool QWaylandWindow::shouldCreateSubSurface() const
{
    return QPlatformWindow::parent() != nullptr;
}

void QWaylandWindow::reset(bool sendDestroyEvent)
{
    if (isInitialized() && sendDestroyEvent) {
        QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed);
        QGuiApplication::sendEvent(window(), &e);
    }
    delete mShellSurface;
    mShellSurface = nullptr;
    delete mSubSurfaceWindow;
    mSubSurfaceWindow = nullptr;
    if (isInitialized())
        destroy();
    mScreens.clear();

    if (mFrameCallback) {
        wl_callback_destroy(mFrameCallback);
        mFrameCallback = nullptr;
    }

    mMask = QRegion();
}

QWaylandWindow *QWaylandWindow::fromWlSurface(::wl_surface *surface)
{
    return static_cast<QWaylandWindow *>(static_cast<QtWayland::wl_surface *>(wl_surface_get_user_data(surface)));
}

WId QWaylandWindow::winId() const
{
    return mWindowId;
}

void QWaylandWindow::setParent(const QPlatformWindow *parent)
{
    if (!window()->isVisible())
        return;

    QWaylandWindow *oldparent = mSubSurfaceWindow ? mSubSurfaceWindow->parent() : nullptr;
    if (oldparent == parent)
        return;

    if (mSubSurfaceWindow && parent) { // new parent, but we were a subsurface already
        delete mSubSurfaceWindow;
        QWaylandWindow *p = const_cast<QWaylandWindow *>(static_cast<const QWaylandWindow *>(parent));
        mSubSurfaceWindow = new QWaylandSubSurface(this, p, mDisplay->createSubSurface(this, p));
    } else { // we're changing role, need to make a new wl_surface
        reset();
        initWindow();
    }
}

void QWaylandWindow::setWindowTitle(const QString &title)
{
    if (mShellSurface) {
        const QString separator = QString::fromUtf8(" \xe2\x80\x94 "); // unicode character U+2014, EM DASH
        const QString formatted = formatWindowTitle(title, separator);

        const int libwaylandMaxBufferSize = 4096;
        // Some parts of the buffer is used for metadata, so subtract 100 to be on the safe side
        const int maxLength = libwaylandMaxBufferSize - 100;

        auto truncated = QStringRef(&formatted).left(maxLength);
        if (truncated.length() < formatted.length()) {
            qCWarning(lcQpaWayland) << "Window titles longer than" << maxLength << "characters are not supported."
                                    << "Truncating window title (from" << formatted.length() << "chars)";
        }
        mShellSurface->setTitle(truncated.toString());
    }

    if (mWindowDecoration && window()->isVisible())
        mWindowDecoration->update();
}

void QWaylandWindow::setWindowIcon(const QIcon &icon)
{
    mWindowIcon = icon;

    if (mWindowDecoration && window()->isVisible())
        mWindowDecoration->update();
}

void QWaylandWindow::setGeometry_helper(const QRect &rect)
{
    QPlatformWindow::setGeometry(QRect(rect.x(), rect.y(),
                qBound(window()->minimumWidth(), rect.width(), window()->maximumWidth()),
                qBound(window()->minimumHeight(), rect.height(), window()->maximumHeight())));

    if (mSubSurfaceWindow) {
        QMargins m = QPlatformWindow::parent()->frameMargins();
        mSubSurfaceWindow->set_position(rect.x() + m.left(), rect.y() + m.top());
        mSubSurfaceWindow->parent()->window()->requestUpdate();
    }
}

void QWaylandWindow::setGeometry(const QRect &rect)
{
    setGeometry_helper(rect);

    if (window()->isVisible() && rect.isValid()) {
        if (mWindowDecoration)
            mWindowDecoration->update();

        if (mResizeAfterSwap && windowType() == Egl && mSentInitialResize)
            mResizeDirty = true;
        else
            QWindowSystemInterface::handleGeometryChange(window(), geometry());

        mSentInitialResize = true;
    }
    sendExposeEvent(QRect(QPoint(), geometry().size()));
}



void QWaylandWindow::sendExposeEvent(const QRect &rect)
{
    if (!(mShellSurface && mShellSurface->handleExpose(rect)))
        QWindowSystemInterface::handleExposeEvent(window(), rect);
}


static QVector<QPointer<QWaylandWindow>> activePopups;

void QWaylandWindow::closePopups(QWaylandWindow *parent)
{
    while (!activePopups.isEmpty()) {
        auto popup = activePopups.takeLast();
        if (popup.isNull())
            continue;
        if (popup.data() == parent)
            return;
        popup->reset();
    }
}

QWaylandScreen *QWaylandWindow::calculateScreenFromSurfaceEvents() const
{
    return mScreens.isEmpty() ? waylandScreen() : mScreens.first();
}

void QWaylandWindow::setVisible(bool visible)
{
    if (visible) {
        if (window()->type() == Qt::Popup)
            activePopups << this;
        initWindow();
        mDisplay->flushRequests();

        setGeometry(window()->geometry());
        // Don't flush the events here, or else the newly visible window may start drawing, but since
        // there was no frame before it will be stuck at the waitForFrameSync() in
        // QWaylandShmBackingStore::beginPaint().
    } else {
        sendExposeEvent(QRect());
        // when flushing the event queue, it could contain a close event, in which
        // case 'this' will be deleted. When that happens, we must abort right away.
        QPointer<QWaylandWindow> deleteGuard(this);
        QWindowSystemInterface::flushWindowSystemEvents();
        if (!deleteGuard.isNull() && window()->type() == Qt::Popup)
            closePopups(this);
        if (!deleteGuard.isNull())
            reset();
    }
}


void QWaylandWindow::raise()
{
    if (mShellSurface)
        mShellSurface->raise();
}


void QWaylandWindow::lower()
{
    if (mShellSurface)
        mShellSurface->lower();
}

void QWaylandWindow::setMask(const QRegion &mask)
{
    if (mMask == mask)
        return;

    mMask = mask;

    if (!isInitialized())
        return;

    if (mMask.isEmpty()) {
        set_input_region(nullptr);
    } else {
        struct ::wl_region *region = mDisplay->createRegion(mMask);
        set_input_region(region);
        wl_region_destroy(region);
    }

    wl_surface::commit();
}

void QWaylandWindow::configure(uint32_t edges, int32_t width, int32_t height)
{
    QMutexLocker resizeLocker(&mResizeLock);
    mConfigure.edges |= edges;
    mConfigure.width = width;
    mConfigure.height = height;

    if (!mRequestResizeSent && !mConfigure.isEmpty()) {
        mRequestResizeSent= true;
        QMetaObject::invokeMethod(this, "requestResize", Qt::QueuedConnection);
    }
}

void QWaylandWindow::doResize()
{
    if (mConfigure.isEmpty()) {
        return;
    }

    int widthWithoutMargins = qMax(mConfigure.width-(frameMargins().left() +frameMargins().right()),1);
    int heightWithoutMargins = qMax(mConfigure.height-(frameMargins().top()+frameMargins().bottom()),1);

    widthWithoutMargins = qMax(widthWithoutMargins, window()->minimumSize().width());
    heightWithoutMargins = qMax(heightWithoutMargins, window()->minimumSize().height());
    QRect geometry = QRect(0,0, widthWithoutMargins, heightWithoutMargins);

    int x = 0;
    int y = 0;
    QSize size = this->geometry().size();
    if (mConfigure.edges & WL_SHELL_SURFACE_RESIZE_LEFT) {
        x = size.width() - geometry.width();
    }
    if (mConfigure.edges & WL_SHELL_SURFACE_RESIZE_TOP) {
        y = size.height() - geometry.height();
    }
    mOffset += QPoint(x, y);

    setGeometry(geometry);

    mConfigure.clear();
}

void QWaylandWindow::setCanResize(bool canResize)
{
    QMutexLocker lock(&mResizeLock);
    mCanResize = canResize;

    if (canResize) {
        if (mResizeDirty) {
            QWindowSystemInterface::handleGeometryChange(window(), geometry());
        }
        if (!mConfigure.isEmpty()) {
            doResize();
            sendExposeEvent(QRect(QPoint(), geometry().size()));
        } else if (mResizeDirty) {
            mResizeDirty = false;
            sendExposeEvent(QRect(QPoint(), geometry().size()));
        }
    }
}

void QWaylandWindow::requestResize()
{
    QMutexLocker lock(&mResizeLock);

    if (mCanResize || !mSentInitialResize) {
        doResize();
    }

    mRequestResizeSent = false;
    lock.unlock();
    sendExposeEvent(QRect(QPoint(), geometry().size()));
    QWindowSystemInterface::flushWindowSystemEvents();
}

void QWaylandWindow::surface_enter(wl_output *output)
{
    QWaylandScreen *oldScreen = calculateScreenFromSurfaceEvents();
    auto addedScreen = QWaylandScreen::fromWlOutput(output);

    if (mScreens.contains(addedScreen)) {
        qWarning() << "Unexpected wl_surface.enter received for output with id:"
                   << wl_proxy_get_id(reinterpret_cast<wl_proxy *>(output))
                   << "screen name:" << addedScreen->name() << "screen model:" << addedScreen->model();
        return;
    }

    mScreens.append(addedScreen);

    QWaylandScreen *newScreen = calculateScreenFromSurfaceEvents();
    if (oldScreen != newScreen) //currently this will only happen if the first wl_surface.enter is for a non-primary screen
        handleScreenChanged();
}

void QWaylandWindow::surface_leave(wl_output *output)
{
    QWaylandScreen *oldScreen = calculateScreenFromSurfaceEvents();
    auto *removedScreen = QWaylandScreen::fromWlOutput(output);
    bool wasRemoved = mScreens.removeOne(removedScreen);
    if (!wasRemoved) {
        qWarning() << "Unexpected wl_surface.leave received for output with id:"
                   << wl_proxy_get_id(reinterpret_cast<wl_proxy *>(output))
                   << "screen name:" << removedScreen->name() << "screen model:" << removedScreen->model();
        return;
    }

    QWaylandScreen *newScreen = calculateScreenFromSurfaceEvents();
    if (oldScreen != newScreen)
        handleScreenChanged();
}

void QWaylandWindow::handleScreenRemoved(QScreen *qScreen)
{
    QWaylandScreen *oldScreen = calculateScreenFromSurfaceEvents();
    bool wasRemoved = mScreens.removeOne(static_cast<QWaylandScreen *>(qScreen->handle()));
    if (wasRemoved) {
        QWaylandScreen *newScreen = calculateScreenFromSurfaceEvents();
        if (oldScreen != newScreen)
            handleScreenChanged();
    }
}

void QWaylandWindow::attach(QWaylandBuffer *buffer, int x, int y)
{
    Q_ASSERT(!buffer->committed());
    if (mFrameCallback) {
        wl_callback_destroy(mFrameCallback);
        mFrameCallback = nullptr;
    }

    if (buffer) {
        mFrameCallback = frame();
        wl_callback_add_listener(mFrameCallback, &QWaylandWindow::callbackListener, this);
        mWaitingForFrameSync = true;
        buffer->setBusy();

        attach(buffer->buffer(), x, y);
    } else {
        QtWayland::wl_surface::attach(nullptr, 0, 0);
    }
}

void QWaylandWindow::attachOffset(QWaylandBuffer *buffer)
{
    attach(buffer, mOffset.x(), mOffset.y());
    mOffset = QPoint();
}

void QWaylandWindow::damage(const QRect &rect)
{
    damage(rect.x(), rect.y(), rect.width(), rect.height());
}

void QWaylandWindow::commit(QWaylandBuffer *buffer, const QRegion &damage)
{
    if (buffer->committed()) {
        qCDebug(lcWaylandBackingstore) << "Buffer already committed, ignoring.";
        return;
    }
    if (!isInitialized())
        return;

    attachOffset(buffer);
    for (const QRect &rect: damage)
        wl_surface::damage(rect.x(), rect.y(), rect.width(), rect.height());
    Q_ASSERT(!buffer->committed());
    buffer->setCommitted();
    wl_surface::commit();
}

const wl_callback_listener QWaylandWindow::callbackListener = {
    QWaylandWindow::frameCallback
};

void QWaylandWindow::frameCallback(void *data, struct wl_callback *callback, uint32_t time)
{
    Q_UNUSED(time);
    Q_UNUSED(callback);
    QWaylandWindow *self = static_cast<QWaylandWindow*>(data);

    self->mWaitingForFrameSync = false;
    if (self->mUpdateRequested) {
        QWindowPrivate *w = QWindowPrivate::get(self->window());
        self->mUpdateRequested = false;
        w->deliverUpdateRequest();
    }
}

QMutex QWaylandWindow::mFrameSyncMutex;

void QWaylandWindow::waitForFrameSync()
{
    QMutexLocker locker(&mFrameSyncMutex);
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

QWaylandSubSurface *QWaylandWindow::subSurfaceWindow() const
{
    return mSubSurfaceWindow;
}

QWaylandScreen *QWaylandWindow::waylandScreen() const
{
    return static_cast<QWaylandScreen *>(QPlatformWindow::screen());
}

void QWaylandWindow::handleContentOrientationChange(Qt::ScreenOrientation orientation)
{
    if (mDisplay->compositorVersion() < 2)
        return;

    wl_output_transform transform;
    bool isPortrait = window()->screen() && window()->screen()->primaryOrientation() == Qt::PortraitOrientation;
    switch (orientation) {
        case Qt::PrimaryOrientation:
            transform = WL_OUTPUT_TRANSFORM_NORMAL;
            break;
        case Qt::LandscapeOrientation:
            transform = isPortrait ? WL_OUTPUT_TRANSFORM_270 : WL_OUTPUT_TRANSFORM_NORMAL;
            break;
        case Qt::PortraitOrientation:
            transform = isPortrait ? WL_OUTPUT_TRANSFORM_NORMAL : WL_OUTPUT_TRANSFORM_90;
            break;
        case Qt::InvertedLandscapeOrientation:
            transform = isPortrait ? WL_OUTPUT_TRANSFORM_90 : WL_OUTPUT_TRANSFORM_180;
            break;
        case Qt::InvertedPortraitOrientation:
            transform = isPortrait ? WL_OUTPUT_TRANSFORM_180 : WL_OUTPUT_TRANSFORM_270;
            break;
        default:
            Q_UNREACHABLE();
    }
    set_buffer_transform(transform);
    // set_buffer_transform is double buffered, we need to commit.
    wl_surface::commit();
}

void QWaylandWindow::setOrientationMask(Qt::ScreenOrientations mask)
{
    if (mShellSurface)
        mShellSurface->setContentOrientationMask(mask);
}

void QWaylandWindow::setWindowState(Qt::WindowStates state)
{
    if (setWindowStateInternal(state))
        QWindowSystemInterface::flushWindowSystemEvents(); // Required for oldState to work on WindowStateChanged
}

void QWaylandWindow::setWindowFlags(Qt::WindowFlags flags)
{
    if (mShellSurface)
        mShellSurface->setWindowFlags(flags);

    mFlags = flags;
    createDecoration();
}

bool QWaylandWindow::createDecoration()
{
    // so far only xdg-shell support this "unminimize" trick, may be moved elsewhere
    if (mState & Qt::WindowMinimized) {
        QWaylandXdgSurface *xdgSurface = qobject_cast<QWaylandXdgSurface *>(mShellSurface);
        if ( xdgSurface ) {
            Qt::WindowStates states;
            if (xdgSurface->isFullscreen())
                states |= Qt::WindowFullScreen;
            if (xdgSurface->isMaximized())
                states |= Qt::WindowMaximized;

            setWindowStateInternal(states);
        }
    }

    if (!mDisplay->supportsWindowDecoration())
        return false;

    static bool decorationPluginFailed = false;
    bool decoration = false;
    switch (window()->type()) {
        case Qt::Window:
        case Qt::Widget:
        case Qt::Dialog:
        case Qt::Tool:
        case Qt::Drawer:
            decoration = true;
            break;
        default:
            break;
    }
    if (mFlags & Qt::FramelessWindowHint || isFullscreen())
        decoration = false;
    if (mFlags & Qt::BypassWindowManagerHint)
        decoration = false;
    if (mSubSurfaceWindow)
        decoration = false;

    bool hadDecoration = mWindowDecoration;
    if (decoration && !decorationPluginFailed) {
        if (!mWindowDecoration) {
            QStringList decorations = QWaylandDecorationFactory::keys();
            if (decorations.empty()) {
                qWarning() << "No decoration plugins available. Running with no decorations.";
                decorationPluginFailed = true;
                return false;
            }

            QString targetKey;
            QByteArray decorationPluginName = qgetenv("QT_WAYLAND_DECORATION");
            if (!decorationPluginName.isEmpty()) {
                targetKey = QString::fromLocal8Bit(decorationPluginName);
                if (!decorations.contains(targetKey)) {
                    qWarning() << "Requested decoration " << targetKey << " not found, falling back to default";
                    targetKey = QString(); // fallthrough
                }
            }

            if (targetKey.isEmpty())
                targetKey = decorations.first(); // first come, first served.


            mWindowDecoration = QWaylandDecorationFactory::create(targetKey, QStringList());
            if (!mWindowDecoration) {
                qWarning() << "Could not create decoration from factory! Running with no decorations.";
                decorationPluginFailed = true;
                return false;
            }
            mWindowDecoration->setWaylandWindow(this);
        }
    } else {
        delete mWindowDecoration;
        mWindowDecoration = nullptr;
    }

    if (hadDecoration != (bool)mWindowDecoration) {
        foreach (QWaylandSubSurface *subsurf, mChildren) {
            QPoint pos = subsurf->window()->geometry().topLeft();
            QMargins m = frameMargins();
            subsurf->set_position(pos.x() + m.left(), pos.y() + m.top());
        }
        sendExposeEvent(QRect(QPoint(), geometry().size()));
    }

    return mWindowDecoration;
}

QWaylandAbstractDecoration *QWaylandWindow::decoration() const
{
    return mWindowDecoration;
}

static QWaylandWindow *closestShellSurfaceWindow(QWindow *window)
{
    while (window) {
        auto w = static_cast<QWaylandWindow *>(window->handle());
        if (w && w->shellSurface())
            return w;
        window = window->transientParent() ? window->transientParent() : window->parent();
    }
    return nullptr;
}

QWaylandWindow *QWaylandWindow::transientParent() const
{
    // Take the closest window with a shell surface, since the transient parent may be a
    // QWidgetWindow or some other window without a shell surface, which is then not able to
    // get mouse events.
    if (auto transientParent = closestShellSurfaceWindow(window()->transientParent()))
        return transientParent;

    if (QGuiApplication::focusWindow() && (window()->type() == Qt::ToolTip || window()->type() == Qt::Popup))
        return closestShellSurfaceWindow(QGuiApplication::focusWindow());

    return nullptr;
}

void QWaylandWindow::handleMouse(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e)
{

    if (mWindowDecoration) {
        handleMouseEventWithDecoration(inputDevice, e);
    } else {
        switch (e.type) {
            case QWaylandPointerEvent::Enter:
                QWindowSystemInterface::handleEnterEvent(window(), e.local, e.global);
                break;
            case QWaylandPointerEvent::Motion:
                QWindowSystemInterface::handleMouseEvent(window(), e.timestamp, e.local, e.global, e.buttons, e.modifiers);
                break;
            case QWaylandPointerEvent::Wheel:
                QWindowSystemInterface::handleWheelEvent(window(), e.timestamp, e.local, e.global, e.pixelDelta, e.angleDelta, e.modifiers);
                break;
        }
    }

#if QT_CONFIG(cursor)
    if (e.type == QWaylandPointerEvent::Enter)
        restoreMouseCursor(inputDevice);
#endif
}

void QWaylandWindow::handleMouseLeave(QWaylandInputDevice *inputDevice)
{
    if (mWindowDecoration) {
        if (mMouseEventsInContentArea) {
            QWindowSystemInterface::handleLeaveEvent(window());
        }
    } else {
        QWindowSystemInterface::handleLeaveEvent(window());
    }
#if QT_CONFIG(cursor)
    restoreMouseCursor(inputDevice);
#endif
}

bool QWaylandWindow::touchDragDecoration(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::TouchPointState state, Qt::KeyboardModifiers mods)
{
    if (!mWindowDecoration)
        return false;
    return mWindowDecoration->handleTouch(inputDevice, local, global, state, mods);
}

void QWaylandWindow::handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e)
{
    if (mMousePressedInContentArea == Qt::NoButton &&
        mWindowDecoration->handleMouse(inputDevice, e.local, e.global, e.buttons, e.modifiers)) {
        if (mMouseEventsInContentArea)
            QWindowSystemInterface::handleLeaveEvent(window());
        return;
    }

    QMargins marg = frameMargins();
    QRect windowRect(0 + marg.left(),
                     0 + marg.top(),
                     geometry().size().width() - marg.right(),
                     geometry().size().height() - marg.bottom());
    if (windowRect.contains(e.local.toPoint()) || mMousePressedInContentArea != Qt::NoButton) {
        QPointF localTranslated = e.local;
        QPointF globalTranslated = e.global;
        localTranslated.setX(localTranslated.x() - marg.left());
        localTranslated.setY(localTranslated.y() - marg.top());
        globalTranslated.setX(globalTranslated.x() - marg.left());
        globalTranslated.setY(globalTranslated.y() - marg.top());
        if (!mMouseEventsInContentArea) {
#if QT_CONFIG(cursor)
            restoreMouseCursor(inputDevice);
#endif
            QWindowSystemInterface::handleEnterEvent(window());
        }

        switch (e.type) {
            case QWaylandPointerEvent::Enter:
                QWindowSystemInterface::handleEnterEvent(window(), localTranslated, globalTranslated);
                break;
            case QWaylandPointerEvent::Motion:
                QWindowSystemInterface::handleMouseEvent(window(), e.timestamp, localTranslated, globalTranslated, e.buttons, e.modifiers);
                break;
            case QWaylandPointerEvent::Wheel:
                QWindowSystemInterface::handleWheelEvent(window(), e.timestamp, localTranslated, globalTranslated, e.pixelDelta, e.angleDelta, e.modifiers);
                break;
        }

        mMouseEventsInContentArea = true;
        mMousePressedInContentArea = e.buttons;
    } else {
        if (mMouseEventsInContentArea) {
            QWindowSystemInterface::handleLeaveEvent(window());
            mMouseEventsInContentArea = false;
        }
    }
}

void QWaylandWindow::handleScreenChanged()
{
    QWaylandScreen *newScreen = calculateScreenFromSurfaceEvents();
    QWindowSystemInterface::handleWindowScreenChanged(window(), newScreen->QPlatformScreen::screen());

    int scale = newScreen->scale();
    if (scale != mScale) {
        mScale = scale;
        if (isInitialized() && mDisplay->compositorVersion() >= 3)
            set_buffer_scale(mScale);
        ensureSize();
    }
}

#if QT_CONFIG(cursor)
void QWaylandWindow::setMouseCursor(QWaylandInputDevice *device, const QCursor &cursor)
{
    device->setCursor(cursor, waylandScreen());
}

void QWaylandWindow::restoreMouseCursor(QWaylandInputDevice *device)
{
    setMouseCursor(device, window()->cursor());
}
#endif

void QWaylandWindow::requestActivateWindow()
{
    qCWarning(lcQpaWayland) << "Wayland does not support QWindow::requestActivate()";
}

void QWaylandWindow::unfocus()
{
#if QT_CONFIG(clipboard)
    QWaylandInputDevice *inputDevice = mDisplay->currentInputDevice();
    if (inputDevice && inputDevice->dataDevice()) {
        inputDevice->dataDevice()->invalidateSelectionOffer();
    }
#endif
}

bool QWaylandWindow::isExposed() const
{
    if (mShellSurface)
        return window()->isVisible() && mShellSurface->isExposed();
    return QPlatformWindow::isExposed();
}

int QWaylandWindow::scale() const
{
    return mScale;
}

qreal QWaylandWindow::devicePixelRatio() const
{
    return mScale;
}

bool QWaylandWindow::setMouseGrabEnabled(bool grab)
{
    if (window()->type() != Qt::Popup) {
        qWarning("This plugin supports grabbing the mouse only for popup windows");
        return false;
    }

    mMouseGrab = grab ? this : nullptr;
    return true;
}

bool QWaylandWindow::setWindowStateInternal(Qt::WindowStates state)
{
    if (mState == state) {
        return false;
    }

    // As of february 2013 QWindow::setWindowState sets the new state value after
    // QPlatformWindow::setWindowState returns, so we cannot rely on QWindow::windowState
    // here. We use then this mState variable.
    mState = state;

    if (mShellSurface) {
        createDecoration();
        if (state & Qt::WindowMaximized)
            mShellSurface->setMaximized();
        if (state & Qt::WindowFullScreen)
            mShellSurface->setFullscreen();
        if (state & Qt::WindowMinimized)
            mShellSurface->setMinimized();
        if (!state)
            mShellSurface->setNormal();
    }

    QWindowSystemInterface::handleWindowStateChanged(window(), mState);
    return true;
}

void QWaylandWindow::sendProperty(const QString &name, const QVariant &value)
{
    m_properties.insert(name, value);
    QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(
                QGuiApplication::platformNativeInterface());
    nativeInterface->emitWindowPropertyChanged(this, name);
    if (mShellSurface)
        mShellSurface->sendProperty(name, value);
}

void QWaylandWindow::setProperty(const QString &name, const QVariant &value)
{
    m_properties.insert(name, value);
    QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(
                QGuiApplication::platformNativeInterface());
    nativeInterface->emitWindowPropertyChanged(this, name);
}

QVariantMap QWaylandWindow::properties() const
{
    return m_properties;
}

QVariant QWaylandWindow::property(const QString &name)
{
    return m_properties.value(name);
}

QVariant QWaylandWindow::property(const QString &name, const QVariant &defaultValue)
{
    return m_properties.value(name, defaultValue);
}

void QWaylandWindow::requestUpdate()
{
    if (!mWaitingForFrameSync)
        QPlatformWindow::requestUpdate();
    else
        mUpdateRequested = true;
}

void QWaylandWindow::addAttachOffset(const QPoint point)
{
    mOffset += point;
}

bool QtWaylandClient::QWaylandWindow::startSystemMove(const QPoint &pos)
{
    Q_UNUSED(pos);
    if (auto seat = display()->lastInputDevice())
        return mShellSurface && mShellSurface->move(seat);
    return false;
}

}

QT_END_NAMESPACE
