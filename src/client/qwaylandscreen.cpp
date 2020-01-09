/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylandscreen_p.h"

#include "qwaylanddisplay_p.h"
#include "qwaylandintegration_p.h"
#include "qwaylandcursor_p.h"
#include "qwaylandwindow_p.h"

#include <QtGui/QGuiApplication>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformwindow.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXdgOutputManagerV1::QWaylandXdgOutputManagerV1(QWaylandDisplay* display, uint id, uint version)
    : QtWayland::zxdg_output_manager_v1(display->wl_registry(), id, qMin(3u, version))
    , m_version(qMin(3u, version))
{
}

QWaylandScreen::QWaylandScreen(QWaylandDisplay *waylandDisplay, int version, uint32_t id)
    : QtWayland::wl_output(waylandDisplay->wl_registry(), id, qMin(version, 2))
    , m_outputId(id)
    , mWaylandDisplay(waylandDisplay)
    , mOutputName(QStringLiteral("Screen%1").arg(id))
{
    if (auto *xdgOutputManager = waylandDisplay->xdgOutputManager())
        initXdgOutput(xdgOutputManager);

    if (version < WL_OUTPUT_DONE_SINCE_VERSION) {
        qCWarning(lcQpaWayland) << "wl_output done event not supported by compositor,"
                                << "QScreen may not work correctly";
        mWaylandDisplay->forceRoundTrip(); // Give the compositor a chance to send geometry etc.
        mOutputDone = true; // Fake the done event
        maybeInitialize();
    }
}

QWaylandScreen::~QWaylandScreen()
{
    if (zxdg_output_v1::isInitialized())
        zxdg_output_v1::destroy();
}

void QWaylandScreen::maybeInitialize()
{
    Q_ASSERT(!mInitialized);

    if (!mOutputDone)
        return;

    if (mWaylandDisplay->xdgOutputManager() && !mXdgOutputDone)
        return;

    mInitialized = true;
    mWaylandDisplay->handleScreenInitialized(this);

    updateOutputProperties();
    if (zxdg_output_v1::isInitialized())
        updateXdgOutputProperties();
}

void QWaylandScreen::initXdgOutput(QWaylandXdgOutputManagerV1 *xdgOutputManager)
{
    Q_ASSERT(xdgOutputManager);
    if (zxdg_output_v1::isInitialized())
        return;

    zxdg_output_v1::init(xdgOutputManager->get_xdg_output(wl_output::object()));
}

QWaylandDisplay * QWaylandScreen::display() const
{
    return mWaylandDisplay;
}

QString QWaylandScreen::manufacturer() const
{
    return mManufacturer;
}

QString QWaylandScreen::model() const
{
    return mModel;
}

QRect QWaylandScreen::geometry() const
{
    if (zxdg_output_v1::isInitialized()) {
        return mXdgGeometry;
    } else {
        // Scale geometry for QScreen. This makes window and screen
        // geometry be in the same coordinate system.
        return QRect(mGeometry.topLeft(), mGeometry.size() / mScale);
    }
}

int QWaylandScreen::depth() const
{
    return mDepth;
}

QImage::Format QWaylandScreen::format() const
{
    return mFormat;
}

QSizeF QWaylandScreen::physicalSize() const
{
    if (mPhysicalSize.isEmpty())
        return QPlatformScreen::physicalSize();
    else
        return mPhysicalSize;
}

QDpi QWaylandScreen::logicalDpi() const
{
    static bool physicalDpi = qEnvironmentVariable("QT_WAYLAND_FORCE_DPI") == QStringLiteral("physical");
    if (physicalDpi)
        return QPlatformScreen::logicalDpi();

    static int forceDpi = qgetenv("QT_WAYLAND_FORCE_DPI").toInt();
    if (forceDpi)
        return QDpi(forceDpi, forceDpi);

    return QDpi(96, 96);
}

QList<QPlatformScreen *> QWaylandScreen::virtualSiblings() const
{
    QList<QPlatformScreen *> list;
    const QList<QWaylandScreen*> screens = mWaylandDisplay->screens();
    auto *placeholder = mWaylandDisplay->placeholderScreen();

    list.reserve(screens.count() + (placeholder ? 1 : 0));

    for (QWaylandScreen *screen : qAsConst(screens)) {
        if (screen->screen())
            list << screen;
    }

    if (placeholder)
        list << placeholder;

    return list;
}

void QWaylandScreen::setOrientationUpdateMask(Qt::ScreenOrientations mask)
{
    const auto allWindows = QGuiApplication::allWindows();
    for (QWindow *window : allWindows) {
        QWaylandWindow *w = static_cast<QWaylandWindow *>(window->handle());
        if (w && w->waylandScreen() == this)
            w->setOrientationMask(mask);
    }
}

Qt::ScreenOrientation QWaylandScreen::orientation() const
{
    return m_orientation;
}

int QWaylandScreen::scale() const
{
    return mScale;
}

qreal QWaylandScreen::devicePixelRatio() const
{
    return qreal(mScale);
}

qreal QWaylandScreen::refreshRate() const
{
    return mRefreshRate / 1000.f;
}

#if QT_CONFIG(cursor)
QPlatformCursor *QWaylandScreen::cursor() const
{
    return mWaylandDisplay->waylandCursor();
}
#endif // QT_CONFIG(cursor)

QWaylandScreen *QWaylandScreen::waylandScreenFromWindow(QWindow *window)
{
    QPlatformScreen *platformScreen = QPlatformScreen::platformScreenForWindow(window);
    if (platformScreen->isPlaceholder())
        return nullptr;
    return static_cast<QWaylandScreen *>(platformScreen);
}

QWaylandScreen *QWaylandScreen::fromWlOutput(::wl_output *output)
{
    if (auto *o = QtWayland::wl_output::fromObject(output))
        return static_cast<QWaylandScreen *>(o);
    return nullptr;
}

void QWaylandScreen::output_mode(uint32_t flags, int width, int height, int refresh)
{
    if (!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    QSize size(width, height);
    if (size != mGeometry.size())
        mGeometry.setSize(size);

    if (refresh != mRefreshRate)
        mRefreshRate = refresh;
}

void QWaylandScreen::output_geometry(int32_t x, int32_t y,
                                     int32_t width, int32_t height,
                                     int subpixel,
                                     const QString &make,
                                     const QString &model,
                                     int32_t transform)
{
    Q_UNUSED(subpixel);

    mManufacturer = make;
    mModel = model;

    mTransform = transform;

    mPhysicalSize = QSize(width, height);
    mGeometry.moveTopLeft(QPoint(x, y));
}

void QWaylandScreen::output_scale(int32_t factor)
{
    mScale = factor;
}

void QWaylandScreen::output_done()
{
    mOutputDone = true;
    if (zxdg_output_v1::isInitialized() && mWaylandDisplay->xdgOutputManager()->version() >= 3)
        mXdgOutputDone = true;
    if (mInitialized) {
        updateOutputProperties();
        if (zxdg_output_v1::isInitialized())
            updateXdgOutputProperties();
    } else {
        maybeInitialize();
    }
}

void QWaylandScreen::updateOutputProperties()
{
    if (mTransform >= 0) {
        bool isPortrait = mGeometry.height() > mGeometry.width();
        switch (mTransform) {
            case WL_OUTPUT_TRANSFORM_NORMAL:
                m_orientation = isPortrait ? Qt::PortraitOrientation : Qt::LandscapeOrientation;
                break;
            case WL_OUTPUT_TRANSFORM_90:
                m_orientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
                break;
            case WL_OUTPUT_TRANSFORM_180:
                m_orientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
                break;
            case WL_OUTPUT_TRANSFORM_270:
                m_orientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
                break;
            // Ignore these ones, at least for now
            case WL_OUTPUT_TRANSFORM_FLIPPED:
            case WL_OUTPUT_TRANSFORM_FLIPPED_90:
            case WL_OUTPUT_TRANSFORM_FLIPPED_180:
            case WL_OUTPUT_TRANSFORM_FLIPPED_270:
                break;
        }

        QWindowSystemInterface::handleScreenOrientationChange(screen(), m_orientation);
        mTransform = -1;
    }

    QWindowSystemInterface::handleScreenRefreshRateChange(screen(), refreshRate());

    if (!zxdg_output_v1::isInitialized())
        QWindowSystemInterface::handleScreenGeometryChange(screen(), geometry(), geometry());
}


void QWaylandScreen::zxdg_output_v1_logical_position(int32_t x, int32_t y)
{
    mXdgGeometry.moveTopLeft(QPoint(x, y));
}

void QWaylandScreen::zxdg_output_v1_logical_size(int32_t width, int32_t height)
{
    mXdgGeometry.setSize(QSize(width, height));
}

void QWaylandScreen::zxdg_output_v1_done()
{
    if (Q_UNLIKELY(mWaylandDisplay->xdgOutputManager()->version() >= 3))
        qWarning(lcQpaWayland) << "zxdg_output_v1.done received on version 3 or newer, this is most likely a bug in the compositor";

    mXdgOutputDone = true;
    if (mInitialized)
        updateXdgOutputProperties();
    else
        maybeInitialize();
}

void QWaylandScreen::zxdg_output_v1_name(const QString &name)
{
    mOutputName = name;
}

void QWaylandScreen::updateXdgOutputProperties()
{
    Q_ASSERT(zxdg_output_v1::isInitialized());
    QWindowSystemInterface::handleScreenGeometryChange(screen(), geometry(), geometry());
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
