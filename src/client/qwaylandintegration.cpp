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

#include "qwaylandintegration_p.h"

#include "qwaylanddisplay_p.h"
#include "qwaylandshmwindow_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandinputcontext_p.h"
#include "qwaylandshmbackingstore_p.h"
#include "qwaylandnativeinterface_p.h"
#include "qwaylandclipboard_p.h"
#include "qwaylanddnd_p.h"
#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandscreen_p.h"

#include "QtPlatformSupport/private/qgenericunixfontdatabase_p.h"
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformcursor.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QSocketNotifier>

#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformaccessibility.h>
#include <qpa/qplatforminputcontext.h>

#include "qwaylandhardwareintegration_p.h"
#include "qwaylandclientbufferintegration_p.h"
#include "qwaylandclientbufferintegrationfactory_p.h"

#include "qwaylandserverbufferintegration_p.h"
#include "qwaylandserverbufferintegrationfactory_p.h"

#include "qwaylandshellintegration_p.h"
#include "qwaylandshellintegrationfactory_p.h"

#include "qwaylandinputdeviceintegration_p.h"
#include "qwaylandinputdeviceintegrationfactory_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class GenericWaylandTheme: public QGenericUnixTheme
{
public:
    static QStringList themeNames()
    {
        QStringList result;

        if (QGuiApplication::desktopSettingsAware()) {
            const QByteArray desktopEnvironment = QGuiApplicationPrivate::platformIntegration()->services()->desktopEnvironment();

            if (desktopEnvironment == QByteArrayLiteral("KDE")) {
#ifndef QT_NO_SETTINGS
                result.push_back(QStringLiteral("kde"));
#endif
            } else if (!desktopEnvironment.isEmpty() &&
                desktopEnvironment != QByteArrayLiteral("UNKNOWN") &&
                desktopEnvironment != QByteArrayLiteral("GNOME") &&
                desktopEnvironment != QByteArrayLiteral("UNITY") &&
                desktopEnvironment != QByteArrayLiteral("MATE") &&
                desktopEnvironment != QByteArrayLiteral("XFCE") &&
                desktopEnvironment != QByteArrayLiteral("LXDE"))
                // Ignore X11 desktop environments
                result.push_back(QString::fromLocal8Bit(desktopEnvironment.toLower()));
        }

        if (result.isEmpty())
            result.push_back(QLatin1String(QGenericUnixTheme::name));

        return result;
    }
};

QWaylandIntegration::QWaylandIntegration()
    : mClientBufferIntegration(0)
    , mShellIntegration(Q_NULLPTR)
    , mInputDeviceIntegration(Q_NULLPTR)
    , mFontDb(new QGenericUnixFontDatabase())
    , mNativeInterface(new QWaylandNativeInterface(this))
#ifndef QT_NO_ACCESSIBILITY
    , mAccessibility(new QPlatformAccessibility())
#else
    , mAccessibility(0)
#endif
    , mClientBufferIntegrationInitialized(false)
    , mServerBufferIntegrationInitialized(false)
    , mShellIntegrationInitialized(false)
{
    initializeInputDeviceIntegration();
    mDisplay = new QWaylandDisplay(this);
    mClipboard = new QWaylandClipboard(mDisplay);
    mDrag = new QWaylandDrag(mDisplay);

    QString icStr = QPlatformInputContextFactory::requested();
    if (!icStr.isNull()) {
        mInputContext.reset(QPlatformInputContextFactory::create(icStr));
    } else {
        //try to use the input context using the wl_text_input interface
        QPlatformInputContext *ctx = new QWaylandInputContext(mDisplay);
        mInputContext.reset(ctx);

        //use the traditional way for on screen keyboards for now
        if (!mInputContext.data()->isValid()) {
            ctx = QPlatformInputContextFactory::create();
            mInputContext.reset(ctx);
        }
    }
}

QWaylandIntegration::~QWaylandIntegration()
{
    delete mDrag;
    delete mClipboard;
#ifndef QT_NO_ACCESSIBILITY
    delete mAccessibility;
#endif
    delete mNativeInterface;
    delete mDisplay;
}

QPlatformNativeInterface * QWaylandIntegration::nativeInterface() const
{
    return mNativeInterface;
}

bool QWaylandIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL:
        return mDisplay->clientBufferIntegration();
    case ThreadedOpenGL:
        return mDisplay->clientBufferIntegration() && mDisplay->clientBufferIntegration()->supportsThreadedOpenGL();
    case BufferQueueingOpenGL:
        return true;
    case MultipleWindows:
    case NonFullScreenWindows:
        return true;
    case RasterGLSurface:
        return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
    if ((window->surfaceType() == QWindow::OpenGLSurface || window->surfaceType() == QWindow::RasterGLSurface)
        && mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createEglWindow(window);

    return new QWaylandShmWindow(window);
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    if (mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createPlatformOpenGLContext(context->format(), context->shareHandle());
    return 0;
}
#endif // QT_NO_OPENGL

QPlatformBackingStore *QWaylandIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QWaylandShmBackingStore(window);
}

QAbstractEventDispatcher *QWaylandIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

void QWaylandIntegration::initialize()
{
    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    QObject::connect(dispatcher, SIGNAL(aboutToBlock()), mDisplay, SLOT(flushRequests()));
    QObject::connect(dispatcher, SIGNAL(awake()), mDisplay, SLOT(flushRequests()));

    int fd = wl_display_get_fd(mDisplay->wl_display());
    QSocketNotifier *sn = new QSocketNotifier(fd, QSocketNotifier::Read, mDisplay);
    QObject::connect(sn, SIGNAL(activated(int)), mDisplay, SLOT(flushRequests()));
}

QPlatformFontDatabase *QWaylandIntegration::fontDatabase() const
{
    return mFontDb;
}

QPlatformClipboard *QWaylandIntegration::clipboard() const
{
    return mClipboard;
}

QPlatformDrag *QWaylandIntegration::drag() const
{
    return mDrag;
}

QPlatformInputContext *QWaylandIntegration::inputContext() const
{
    return mInputContext.data();
}

QVariant QWaylandIntegration::styleHint(StyleHint hint) const
{
    if (hint == ShowIsFullScreen && mDisplay->windowManagerIntegration())
        return mDisplay->windowManagerIntegration()->showIsFullScreen();

    switch (hint) {
    case QPlatformIntegration::FontSmoothingGamma:
        return qreal(1.0);
    default:
        break;
    }

    return QPlatformIntegration::styleHint(hint);
}

QPlatformAccessibility *QWaylandIntegration::accessibility() const
{
    return mAccessibility;
}

QPlatformServices *QWaylandIntegration::services() const
{
    return mDisplay->windowManagerIntegration();
}

QWaylandDisplay *QWaylandIntegration::display() const
{
    return mDisplay;
}

QStringList QWaylandIntegration::themeNames() const
{
    return GenericWaylandTheme::themeNames();
}

QPlatformTheme *QWaylandIntegration::createPlatformTheme(const QString &name) const
{
    return GenericWaylandTheme::createUnixTheme(name);
}

QWaylandClientBufferIntegration *QWaylandIntegration::clientBufferIntegration() const
{
    if (!mClientBufferIntegrationInitialized)
        const_cast<QWaylandIntegration *>(this)->initializeClientBufferIntegration();

    return mClientBufferIntegration && mClientBufferIntegration->isValid() ? mClientBufferIntegration : 0;
}

QWaylandServerBufferIntegration *QWaylandIntegration::serverBufferIntegration() const
{
    if (!mServerBufferIntegrationInitialized)
        const_cast<QWaylandIntegration *>(this)->initializeServerBufferIntegration();

    return mServerBufferIntegration;
}

QWaylandShellIntegration *QWaylandIntegration::shellIntegration() const
{
    if (!mShellIntegrationInitialized)
        const_cast<QWaylandIntegration *>(this)->initializeShellIntegration();

    return mShellIntegration;
}

void QWaylandIntegration::initializeClientBufferIntegration()
{
    mClientBufferIntegrationInitialized = true;

    QString targetKey;
    bool disableHardwareIntegration = qEnvironmentVariableIsSet("QT_WAYLAND_DISABLE_HW_INTEGRATION");
    disableHardwareIntegration = disableHardwareIntegration || !mDisplay->hardwareIntegration();
    if (disableHardwareIntegration) {
        QByteArray clientBufferIntegrationName = qgetenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION");
        if (clientBufferIntegrationName.isEmpty())
            clientBufferIntegrationName = QByteArrayLiteral("wayland-egl");
        targetKey = QString::fromLocal8Bit(clientBufferIntegrationName);
    } else {
        targetKey = mDisplay->hardwareIntegration()->clientBufferIntegration();
    }

    if (targetKey.isEmpty()) {
        qWarning("Failed to determine what client buffer integration to use");
        return;
    }

    QStringList keys = QWaylandClientBufferIntegrationFactory::keys();
    if (keys.contains(targetKey)) {
        mClientBufferIntegration = QWaylandClientBufferIntegrationFactory::create(targetKey, QStringList());
    }
    if (mClientBufferIntegration)
        mClientBufferIntegration->initialize(mDisplay);
    else
        qWarning("Failed to load client buffer integration: %s\n", qPrintable(targetKey));
}

void QWaylandIntegration::initializeServerBufferIntegration()
{
    mServerBufferIntegrationInitialized = true;

    QString targetKey;

    bool disableHardwareIntegration = qEnvironmentVariableIsSet("QT_WAYLAND_DISABLE_HW_INTEGRATION");
    disableHardwareIntegration = disableHardwareIntegration || !mDisplay->hardwareIntegration();
    if (disableHardwareIntegration) {
        QByteArray serverBufferIntegrationName = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
        QString targetKey = QString::fromLocal8Bit(serverBufferIntegrationName);
    } else {
        targetKey = mDisplay->hardwareIntegration()->serverBufferIntegration();
    }

    if (targetKey.isEmpty()) {
        qWarning("Failed to determine what server buffer integration to use");
        return;
    }

    QStringList keys = QWaylandServerBufferIntegrationFactory::keys();
    if (keys.contains(targetKey)) {
        mServerBufferIntegration = QWaylandServerBufferIntegrationFactory::create(targetKey, QStringList());
    }
    if (mServerBufferIntegration)
        mServerBufferIntegration->initialize(mDisplay);
    else
        qWarning("Failed to load server buffer integration %s\n", qPrintable(targetKey));
}

void QWaylandIntegration::initializeShellIntegration()
{
    mShellIntegrationInitialized = true;

    QByteArray integrationName = qgetenv("QT_WAYLAND_SHELL_INTEGRATION");
    QString targetKey = QString::fromLocal8Bit(integrationName);

    if (targetKey.isEmpty()) {
        return;
    }

    QStringList keys = QWaylandShellIntegrationFactory::keys();
    if (keys.contains(targetKey)) {
        mShellIntegration = QWaylandShellIntegrationFactory::create(targetKey, QStringList());
    }
    if (mShellIntegration && mShellIntegration->initialize(mDisplay)) {
        qDebug("Using the '%s' shell integration", qPrintable(targetKey));
    } else {
        delete mShellIntegration;
        mShellIntegration = Q_NULLPTR;
        qWarning("Failed to load shell integration %s", qPrintable(targetKey));
    }
}

QWaylandInputDevice *QWaylandIntegration::createInputDevice(QWaylandDisplay *display, int version, uint32_t id)
{
    if (mInputDeviceIntegration) {
        return mInputDeviceIntegration->createInputDevice(display, version, id);
    }
    return new QWaylandInputDevice(display, version, id);
}

void QWaylandIntegration::initializeInputDeviceIntegration()
{
    QByteArray integrationName = qgetenv("QT_WAYLAND_INPUTDEVICE_INTEGRATION");
    QString targetKey = QString::fromLocal8Bit(integrationName);

    if (targetKey.isEmpty()) {
        return;
    }

    QStringList keys = QWaylandInputDeviceIntegrationFactory::keys();
    if (keys.contains(targetKey)) {
        mInputDeviceIntegration = QWaylandInputDeviceIntegrationFactory::create(targetKey, QStringList());
        qDebug("Using the '%s' input device integration", qPrintable(targetKey));
    } else {
        qWarning("Wayland inputdevice integration '%s' not found, using default", qPrintable(targetKey));
    }
}

}

QT_END_NAMESPACE
