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
#include "qwaylandsurface_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandshellsurface_p.h"
#include "qwaylandsubsurface_p.h"
#include "qwaylandabstractdecoration_p.h"
#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandnativeinterface_p.h"
#include "qwaylanddecorationfactory_p.h"
#include "qwaylandshmbackingstore_p.h"
#include "qwaylandshellintegration_p.h"

#include <QtCore/QFileInfo>
#include <QtCore/QPointer>
#include <QtCore/QRegularExpression>
#include <QtGui/QWindow>

#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QtGui/private/qwindow_p.h>

#include <QtCore/QDebug>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_LOGGING_CATEGORY(lcWaylandBackingstore, "qt.qpa.wayland.backingstore")

QWaylandWindow *QWaylandWindow::mMouseGrab = nullptr;

QWaylandWindow::QWaylandWindow(QWindow *window, QWaylandDisplay *display)
    : QPlatformWindow(window)
    , mDisplay(display)
    , mFrameQueue(mDisplay->createFrameQueue())
    , mResizeAfterSwap(qEnvironmentVariableIsSet("QT_WAYLAND_RESIZE_AFTER_SWAP"))
{
    {
        bool ok;
        int frameCallbackTimeout = qEnvironmentVariableIntValue("QT_WAYLAND_FRAME_CALLBACK_TIMEOUT", &ok);
        if (ok)
            mFrameCallbackTimeout = frameCallbackTimeout;
    }

    mScale = waylandScreen() ? waylandScreen()->scale() : 1; // fallback to 1 if we don't have a real screen

    static WId id = 1;
    mWindowId = id++;
    initializeWlSurface();
}

QWaylandWindow::~QWaylandWindow()
{
    mDisplay->destroyFrameQueue(mFrameQueue);
    mDisplay->handleWindowDestroyed(this);

    delete mWindowDecoration;

    if (mSurface)
        reset();

    const QWindow *parent = window();
    const auto tlw = QGuiApplication::topLevelWindows();
    for (QWindow *w : tlw) {
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

    if (!mSurface) {
        initializeWlSurface();
    }

    if (shouldCreateSubSurface()) {
        Q_ASSERT(!mSubSurfaceWindow);

        auto *parent = static_cast<QWaylandWindow *>(QPlatformWindow::parent());
        if (parent->wlSurface()) {
            if (::wl_subsurface *subsurface = mDisplay->createSubSurface(this, parent))
                mSubSurfaceWindow = new QWaylandSubSurface(this, parent, subsurface);
        }
    } else if (shouldCreateShellSurface()) {
        Q_ASSERT(!mShellSurface);
        Q_ASSERT(mDisplay->shellIntegration());

        mShellSurface = mDisplay->shellIntegration()->createShellSurface(this);
        if (mShellSurface) {
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
                                                                                 Qt::SkipEmptyParts);

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

    // Enable high-dpi rendering. Scale() returns the screen scale factor and will
    // typically be integer 1 (normal-dpi) or 2 (high-dpi). Call set_buffer_scale()
    // to inform the compositor that high-resolution buffers will be provided.
    if (mDisplay->compositorVersion() >= 3)
        mSurface->set_buffer_scale(scale());

    if (QScreen *s = window()->screen())
        setOrientationMask(s->orientationUpdateMask());
    setWindowFlags(window()->flags());
    if (window()->geometry().isEmpty())
        setGeometry_helper(QRect(QPoint(), QSize(500,500)));
    else
        setGeometry_helper(window()->geometry());
    setMask(window()->mask());
    if (mShellSurface)
        mShellSurface->requestWindowStates(window()->windowStates());
    handleContentOrientationChange(window()->contentOrientation());
    mFlags = window()->flags();
}

void QWaylandWindow::initializeWlSurface()
{
    Q_ASSERT(!mSurface);
    {
        QWriteLocker lock(&mSurfaceLock);
        mSurface.reset(new QWaylandSurface(mDisplay));
        connect(mSurface.data(), &QWaylandSurface::screensChanged,
                this, &QWaylandWindow::handleScreensChanged);
        mSurface->m_window = this;
    }
    emit wlSurfaceCreated();
}

bool QWaylandWindow::shouldCreateShellSurface() const
{
    if (!mDisplay->shellIntegration())
        return false;

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

void QWaylandWindow::reset()
{
    delete mShellSurface;
    mShellSurface = nullptr;
    delete mSubSurfaceWindow;
    mSubSurfaceWindow = nullptr;

    invalidateSurface();
    if (mSurface) {
        emit wlSurfaceDestroyed();
        QWriteLocker lock(&mSurfaceLock);
        mSurface.reset();
    }

    if (mFrameCallback) {
        wl_callback_destroy(mFrameCallback);
        mFrameCallback = nullptr;
    }

    mFrameCallbackElapsedTimer.invalidate();
    mWaitingForFrameCallback = false;
    mFrameCallbackTimedOut = false;

    mMask = QRegion();
    mQueuedBuffer = nullptr;
}

QWaylandWindow *QWaylandWindow::fromWlSurface(::wl_surface *surface)
{
    if (auto *s = QWaylandSurface::fromWlSurface(surface))
        return s->m_window;
    return nullptr;
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
        // Some parts of the buffer is used for metadata, so subtract 100 to be on the safe side.
        // Also, QString is in utf-16, which means that in the worst case each character will be
        // three bytes when converted to utf-8 (which is what libwayland uses), so divide by three.
        const int maxLength = libwaylandMaxBufferSize / 3 - 100;

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
    QRect exposeGeometry(QPoint(), geometry().size());
    if (isExposed() && !mInResizeFromApplyConfigure && exposeGeometry != mLastExposeGeometry)
        sendExposeEvent(exposeGeometry);

    if (mShellSurface)
        mShellSurface->setWindowGeometry(windowContentGeometry());

    if (isOpaque() && mMask.isEmpty())
        setOpaqueArea(rect);
}

void QWaylandWindow::resizeFromApplyConfigure(const QSize &sizeWithMargins, const QPoint &offset)
{
    QMargins margins = frameMargins();
    int widthWithoutMargins = qMax(sizeWithMargins.width() - (margins.left() + margins.right()), 1);
    int heightWithoutMargins = qMax(sizeWithMargins.height() - (margins.top() + margins.bottom()), 1);
    QRect geometry(windowGeometry().topLeft(), QSize(widthWithoutMargins, heightWithoutMargins));

    mOffset += offset;
    mInResizeFromApplyConfigure = true;
    setGeometry(geometry);
    mInResizeFromApplyConfigure = false;
}

void QWaylandWindow::sendExposeEvent(const QRect &rect)
{
    if (!(mShellSurface && mShellSurface->handleExpose(rect)))
        QWindowSystemInterface::handleExposeEvent(window(), rect);
    else
        qCDebug(lcQpaWayland) << "sendExposeEvent: intercepted by shell extension, not sending";
    mLastExposeGeometry = rect;
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

QPlatformScreen *QWaylandWindow::calculateScreenFromSurfaceEvents() const
{
    if (mSurface) {
        if (auto *screen = mSurface->oldestEnteredScreen())
            return screen;
    }

    return QPlatformWindow::screen();
}

void QWaylandWindow::setVisible(bool visible)
{
    // Workaround for issue where setVisible may be called with the same value twice
    if (lastVisible == visible)
        return;
    lastVisible = visible;

    if (visible) {
        if (window()->type() == Qt::Popup || window()->type() == Qt::ToolTip)
            activePopups << this;
        initWindow();
        mDisplay->flushRequests();

        setGeometry(window()->geometry());
        // Don't flush the events here, or else the newly visible window may start drawing, but since
        // there was no frame before it will be stuck at the waitForFrameSync() in
        // QWaylandShmBackingStore::beginPaint().
    } else {
        sendExposeEvent(QRect());
        closePopups(this);
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

    if (!mSurface)
        return;

    if (mMask.isEmpty()) {
        mSurface->set_input_region(nullptr);

        if (isOpaque())
            setOpaqueArea(QRect(QPoint(0, 0), geometry().size()));
    } else {
        struct ::wl_region *region = mDisplay->createRegion(mMask);
        mSurface->set_input_region(region);
        wl_region_destroy(region);

        if (isOpaque())
            setOpaqueArea(mMask);
    }

    mSurface->commit();
}

void QWaylandWindow::applyConfigureWhenPossible()
{
    QMutexLocker resizeLocker(&mResizeLock);
    if (!mWaitingToApplyConfigure) {
        mWaitingToApplyConfigure = true;
        QMetaObject::invokeMethod(this, "applyConfigure", Qt::QueuedConnection);
    }
}

void QWaylandWindow::doApplyConfigure()
{
    if (!mWaitingToApplyConfigure)
        return;

    if (mShellSurface)
        mShellSurface->applyConfigure();

    mWaitingToApplyConfigure = false;
}

void QWaylandWindow::setCanResize(bool canResize)
{
    QMutexLocker lock(&mResizeLock);
    mCanResize = canResize;

    if (canResize) {
        if (mResizeDirty) {
            QWindowSystemInterface::handleGeometryChange(window(), geometry());
        }
        if (mWaitingToApplyConfigure) {
            doApplyConfigure();
            sendExposeEvent(QRect(QPoint(), geometry().size()));
        } else if (mResizeDirty) {
            mResizeDirty = false;
            sendExposeEvent(QRect(QPoint(), geometry().size()));
        }
    }
}

void QWaylandWindow::applyConfigure()
{
    QMutexLocker lock(&mResizeLock);

    if (mCanResize || !mSentInitialResize)
        doApplyConfigure();

    lock.unlock();
    sendRecursiveExposeEvent();
    QWindowSystemInterface::flushWindowSystemEvents();
}

void QWaylandWindow::sendRecursiveExposeEvent()
{
    if (!window()->isVisible())
        return;
    sendExposeEvent(QRect(QPoint(), geometry().size()));

    for (QWaylandSubSurface *subSurface : qAsConst(mChildren)) {
        auto subWindow = subSurface->window();
        subWindow->sendRecursiveExposeEvent();
    }
}

void QWaylandWindow::attach(QWaylandBuffer *buffer, int x, int y)
{
    Q_ASSERT(!buffer->committed());
    if (buffer) {
        handleUpdate();
        buffer->setBusy();

        mSurface->attach(buffer->buffer(), x, y);
    } else {
        mSurface->attach(nullptr, 0, 0);
    }
}

void QWaylandWindow::attachOffset(QWaylandBuffer *buffer)
{
    attach(buffer, mOffset.x(), mOffset.y());
    mOffset = QPoint();
}

void QWaylandWindow::damage(const QRect &rect)
{
    mSurface->damage(rect.x(), rect.y(), rect.width(), rect.height());
}

void QWaylandWindow::safeCommit(QWaylandBuffer *buffer, const QRegion &damage)
{
    if (isExposed()) {
        commit(buffer, damage);
    } else {
        mQueuedBuffer = buffer;
        mQueuedBufferDamage = damage;
    }
}

void QWaylandWindow::handleExpose(const QRegion &region)
{
    QWindowSystemInterface::handleExposeEvent(window(), region);
    if (mQueuedBuffer) {
        commit(mQueuedBuffer, mQueuedBufferDamage);
        mQueuedBuffer = nullptr;
        mQueuedBufferDamage = QRegion();
    }
}

void QWaylandWindow::commit(QWaylandBuffer *buffer, const QRegion &damage)
{
    Q_ASSERT(isExposed());
    if (buffer->committed()) {
        qCDebug(lcWaylandBackingstore) << "Buffer already committed, ignoring.";
        return;
    }
    if (!mSurface)
        return;

    attachOffset(buffer);
    for (const QRect &rect: damage)
        mSurface->damage(rect.x(), rect.y(), rect.width(), rect.height());
    Q_ASSERT(!buffer->committed());
    buffer->setCommitted();
    mSurface->commit();
}

void QWaylandWindow::commit()
{
    mSurface->commit();
}

const wl_callback_listener QWaylandWindow::callbackListener = {
    [](void *data, wl_callback *callback, uint32_t time) {
        Q_UNUSED(callback);
        Q_UNUSED(time);
        auto *window = static_cast<QWaylandWindow*>(data);
        window->handleFrameCallback();
    }
};

void QWaylandWindow::handleFrameCallback()
{
    mWaitingForFrameCallback = false;
    mFrameCallbackElapsedTimer.invalidate();

    // The rest can wait until we can run it on the correct thread
    if (!mWaitingForUpdateDelivery) {
        auto doHandleExpose = [this]() {
            bool wasExposed = isExposed();
            mFrameCallbackTimedOut = false;
            if (!wasExposed && isExposed()) // Did setting mFrameCallbackTimedOut make the window exposed?
                sendExposeEvent(QRect(QPoint(), geometry().size()));
            if (wasExposed && hasPendingUpdateRequest())
                deliverUpdateRequest();

            mWaitingForUpdateDelivery = false;
        };

        // Queued connection, to make sure we don't call handleUpdate() from inside waitForFrameSync()
        // in the single-threaded case.
        mWaitingForUpdateDelivery = true;
        QMetaObject::invokeMethod(this, doHandleExpose, Qt::QueuedConnection);
    }
}

bool QWaylandWindow::waitForFrameSync(int timeout)
{
    QMutexLocker locker(mFrameQueue.mutex);
    mDisplay->dispatchQueueWhile(mFrameQueue.queue, [&]() { return mWaitingForFrameCallback; }, timeout);

    if (mWaitingForFrameCallback) {
        qCDebug(lcWaylandBackingstore) << "Didn't receive frame callback in time, window should now be inexposed";
        mFrameCallbackTimedOut = true;
        mWaitingForUpdate = false;
        sendExposeEvent(QRect());
    }

    return !mWaitingForFrameCallback;
}

QMargins QWaylandWindow::frameMargins() const
{
    if (mWindowDecoration)
        return mWindowDecoration->margins();
    return QPlatformWindow::frameMargins();
}

/*!
 * Size, with decorations (including including eventual shadows) in wl_surface coordinates
 */
QSize QWaylandWindow::surfaceSize() const
{
    return geometry().marginsAdded(frameMargins()).size();
}

/*!
 * Window geometry as defined by the xdg-shell spec (in wl_surface coordinates)
 * topLeft is where the shadow stops and the decorations border start.
 */
QRect QWaylandWindow::windowContentGeometry() const
{
    return QRect(QPoint(), surfaceSize());
}

/*!
 * Converts from wl_surface coordinates to Qt window coordinates. Qt window
 * coordinates start inside (not including) the window decorations, while
 * wl_surface coordinates start at the first pixel of the buffer. Potentially,
 * this should be in the window shadow, although we don't have those. So for
 * now, it's the first pixel of the decorations.
 */
QPointF QWaylandWindow::mapFromWlSurface(const QPointF &surfacePosition) const
{
    const QMargins margins = frameMargins();
    return QPointF(surfacePosition.x() - margins.left(), surfacePosition.y() - margins.top());
}

wl_surface *QWaylandWindow::wlSurface()
{
    return mSurface ? mSurface->object() : nullptr;
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
    auto *platformScreen = QPlatformWindow::screen();
    Q_ASSERT(platformScreen);
    if (platformScreen->isPlaceholder())
        return nullptr;
    return static_cast<QWaylandScreen *>(platformScreen);
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
    mSurface->set_buffer_transform(transform);
    // set_buffer_transform is double buffered, we need to commit.
    mSurface->commit();
}

void QWaylandWindow::setOrientationMask(Qt::ScreenOrientations mask)
{
    if (mShellSurface)
        mShellSurface->setContentOrientationMask(mask);
}

void QWaylandWindow::setWindowState(Qt::WindowStates states)
{
    if (mShellSurface)
        mShellSurface->requestWindowStates(states);
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
    if (mFlags & Qt::FramelessWindowHint)
        decoration = false;
    if (mFlags & Qt::BypassWindowManagerHint)
        decoration = false;
    if (mSubSurfaceWindow)
        decoration = false;
    if (mShellSurface && !mShellSurface->wantsDecorations())
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
        for (QWaylandSubSurface *subsurf : qAsConst(mChildren)) {
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
    if (e.type == QEvent::Leave) {
        if (mWindowDecoration) {
            if (mMouseEventsInContentArea)
                QWindowSystemInterface::handleLeaveEvent(window());
        } else {
            QWindowSystemInterface::handleLeaveEvent(window());
        }
#if QT_CONFIG(cursor)
        restoreMouseCursor(inputDevice);
#endif
        return;
    }

    if (mWindowDecoration) {
        handleMouseEventWithDecoration(inputDevice, e);
    } else {
        switch (e.type) {
            case QEvent::Enter:
                QWindowSystemInterface::handleEnterEvent(window(), e.local, e.global);
                break;
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
                QWindowSystemInterface::handleMouseEvent(window(), e.timestamp, e.local, e.global, e.buttons, e.button, e.type, e.modifiers);
                break;
            case QEvent::Wheel:
                QWindowSystemInterface::handleWheelEvent(window(), e.timestamp, e.local, e.global,
                                                         e.pixelDelta, e.angleDelta, e.modifiers,
                                                         e.phase, e.source, false);
                break;
        default:
            Q_UNREACHABLE();
        }
    }

#if QT_CONFIG(cursor)
    if (e.type == QEvent::Enter) {
        QRect contentGeometry = windowContentGeometry().marginsRemoved(frameMargins());
        if (contentGeometry.contains(e.local.toPoint()))
            restoreMouseCursor(inputDevice);
    }
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
        if (mMouseEventsInContentArea) {
            QWindowSystemInterface::handleLeaveEvent(window());
            mMouseEventsInContentArea = false;
        }
        return;
    }

    QMargins marg = frameMargins();
    QRect windowRect(0 + marg.left(),
                     0 + marg.top(),
                     geometry().size().width() - marg.right(),
                     geometry().size().height() - marg.bottom());
    if (windowRect.contains(e.local.toPoint()) || mMousePressedInContentArea != Qt::NoButton) {
        const QPointF localTranslated = mapFromWlSurface(e.local);
        QPointF globalTranslated = e.global;
        globalTranslated.setX(globalTranslated.x() - marg.left());
        globalTranslated.setY(globalTranslated.y() - marg.top());
        if (!mMouseEventsInContentArea) {
#if QT_CONFIG(cursor)
            restoreMouseCursor(inputDevice);
#endif
            QWindowSystemInterface::handleEnterEvent(window());
        }

        switch (e.type) {
            case QEvent::Enter:
                QWindowSystemInterface::handleEnterEvent(window(), localTranslated, globalTranslated);
                break;
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
                QWindowSystemInterface::handleMouseEvent(window(), e.timestamp, localTranslated, globalTranslated, e.buttons, e.button, e.type, e.modifiers);
                break;
            case QEvent::Wheel: {
                QWindowSystemInterface::handleWheelEvent(window(), e.timestamp,
                                                         localTranslated, globalTranslated,
                                                         e.pixelDelta, e.angleDelta, e.modifiers,
                                                         e.phase, e.source, false);
                break;
            }
            default:
                Q_UNREACHABLE();
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

void QWaylandWindow::handleScreensChanged()
{
    QPlatformScreen *newScreen = calculateScreenFromSurfaceEvents();

    if (newScreen == mLastReportedScreen)
        return;

    QWindowSystemInterface::handleWindowScreenChanged(window(), newScreen->QPlatformScreen::screen());
    mLastReportedScreen = newScreen;

    int scale = newScreen->isPlaceholder() ? 1 : static_cast<QWaylandScreen *>(newScreen)->scale();
    if (scale != mScale) {
        mScale = scale;
        if (mSurface && mDisplay->compositorVersion() >= 3)
            mSurface->set_buffer_scale(mScale);
        ensureSize();
    }
}

#if QT_CONFIG(cursor)
void QWaylandWindow::setMouseCursor(QWaylandInputDevice *device, const QCursor &cursor)
{
    int fallbackBufferScale = int(devicePixelRatio());
    device->setCursor(&cursor, {}, fallbackBufferScale);
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

bool QWaylandWindow::isExposed() const
{
    if (!window()->isVisible())
        return false;

    if (mFrameCallbackTimedOut)
        return false;

    if (mShellSurface)
        return mShellSurface->isExposed();

    if (mSubSurfaceWindow)
        return mSubSurfaceWindow->parent()->isExposed();

    return !(shouldCreateShellSurface() || shouldCreateSubSurface());
}

bool QWaylandWindow::isActive() const
{
    return mDisplay->isWindowActivated(this);
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

void QWaylandWindow::handleWindowStatesChanged(Qt::WindowStates states)
{
    createDecoration();
    QWindowSystemInterface::handleWindowStateChanged(window(), states, mLastReportedWindowStates);
    mLastReportedWindowStates = states;
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

void QWaylandWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != mFrameCallbackCheckIntervalTimerId)
        return;

    bool callbackTimerExpired = mFrameCallbackElapsedTimer.hasExpired(mFrameCallbackTimeout);
    if (!mFrameCallbackElapsedTimer.isValid() || callbackTimerExpired ) {
        killTimer(mFrameCallbackCheckIntervalTimerId);
        mFrameCallbackCheckIntervalTimerId = -1;
    }
    if (mFrameCallbackElapsedTimer.isValid() && callbackTimerExpired) {
        mFrameCallbackElapsedTimer.invalidate();

        qCDebug(lcWaylandBackingstore) << "Didn't receive frame callback in time, window should now be inexposed";
        mFrameCallbackTimedOut = true;
        mWaitingForUpdate = false;
        sendExposeEvent(QRect());
    }
}

void QWaylandWindow::requestUpdate()
{
    qCDebug(lcWaylandBackingstore) << "requestUpdate";
    Q_ASSERT(hasPendingUpdateRequest()); // should be set by QPA

    // If we have a frame callback all is good and will be taken care of there
    if (mWaitingForFrameCallback)
        return;

    // If we've already called deliverUpdateRequest(), but haven't seen any attach+commit/swap yet
    // This is a somewhat redundant behavior and might indicate a bug in the calling code, so log
    // here so we can get this information when debugging update/frame callback issues.
    // Continue as nothing happened, though.
    if (mWaitingForUpdate)
        qCDebug(lcWaylandBackingstore) << "requestUpdate called twice without committing anything";

    // Some applications (such as Qt Quick) depend on updates being delivered asynchronously,
    // so use invokeMethod to delay the delivery a bit.
    QMetaObject::invokeMethod(this, [this] {
        // Things might have changed in the meantime
        if (hasPendingUpdateRequest() && !mWaitingForFrameCallback)
            deliverUpdateRequest();
    }, Qt::QueuedConnection);
}

// Should be called whenever we commit a buffer (directly through wl_surface.commit or indirectly
// with eglSwapBuffers) to know when it's time to commit the next one.
// Can be called from the render thread (without locking anything) so make sure to not make races in this method.
void QWaylandWindow::handleUpdate()
{
    qCDebug(lcWaylandBackingstore) << "handleUpdate" << QThread::currentThread();
    // TODO: Should sync subsurfaces avoid requesting frame callbacks?
    QReadLocker lock(&mSurfaceLock);
    if (!mSurface)
        return;

    if (mFrameCallback) {
        wl_callback_destroy(mFrameCallback);
        mFrameCallback = nullptr;
    }

    QMutexLocker locker(mFrameQueue.mutex);
    struct ::wl_surface *wrappedSurface = reinterpret_cast<struct ::wl_surface *>(wl_proxy_create_wrapper(mSurface->object()));
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(wrappedSurface), mFrameQueue.queue);
    mFrameCallback = wl_surface_frame(wrappedSurface);
    wl_proxy_wrapper_destroy(wrappedSurface);
    wl_callback_add_listener(mFrameCallback, &QWaylandWindow::callbackListener, this);
    mWaitingForFrameCallback = true;
    mWaitingForUpdate = false;

    // Start a timer for handling the case when the compositor stops sending frame callbacks.
    if (mFrameCallbackTimeout > 0) {
        QMetaObject::invokeMethod(this, [this] {
            if (mWaitingForFrameCallback) {
                if (mFrameCallbackCheckIntervalTimerId < 0)
                    mFrameCallbackCheckIntervalTimerId = startTimer(mFrameCallbackTimeout);
                mFrameCallbackElapsedTimer.start();
            }
        }, Qt::QueuedConnection);
    }
}

void QWaylandWindow::deliverUpdateRequest()
{
    qCDebug(lcWaylandBackingstore) << "deliverUpdateRequest";
    mWaitingForUpdate = true;
    QPlatformWindow::deliverUpdateRequest();
}

void QWaylandWindow::addAttachOffset(const QPoint point)
{
    mOffset += point;
}

void QWaylandWindow::propagateSizeHints()
{
    if (mShellSurface)
        mShellSurface->propagateSizeHints();
}

bool QWaylandWindow::startSystemResize(Qt::Edges edges)
{
    if (auto *seat = display()->lastInputDevice())
        return mShellSurface && mShellSurface->resize(seat, edges);
    return false;
}

bool QtWaylandClient::QWaylandWindow::startSystemMove()
{
    if (auto seat = display()->lastInputDevice())
        return mShellSurface && mShellSurface->move(seat);
    return false;
}

bool QWaylandWindow::isOpaque() const
{
    return window()->requestedFormat().alphaBufferSize() <= 0;
}

void QWaylandWindow::setOpaqueArea(const QRegion &opaqueArea)
{
    if (opaqueArea == mOpaqueArea || !mSurface)
        return;

    mOpaqueArea = opaqueArea;

    struct ::wl_region *region = mDisplay->createRegion(opaqueArea);
    mSurface->set_opaque_region(region);
    wl_region_destroy(region);
}

}

QT_END_NAMESPACE
