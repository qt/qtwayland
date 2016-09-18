/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
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
#ifndef QT_NO_CLIPBOARD
#include "qwaylandclipboard_p.h"
#endif
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
#ifndef QT_NO_ACCESSIBILITY
#include <qpa/qplatformaccessibility.h>
#endif
#include <qpa/qplatforminputcontext.h>

#include "qwaylandhardwareintegration_p.h"
#include "qwaylandclientbufferintegration_p.h"
#include "qwaylandclientbufferintegrationfactory_p.h"

#include "qwaylandserverbufferintegration_p.h"
#include "qwaylandserverbufferintegrationfactory_p.h"

#include "qwaylandshellintegration_p.h"
#include "qwaylandshellintegrationfactory_p.h"
#include "qwaylandxdgshellintegration_p.h"
#include "qwaylandwlshellintegration_p.h"

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
#endif
    , mClientBufferIntegrationInitialized(false)
    , mServerBufferIntegrationInitialized(false)
    , mShellIntegrationInitialized(false)
{
    initializeInputDeviceIntegration();
    mDisplay = new QWaylandDisplay(this);
#ifndef QT_NO_CLIPBOARD
    mClipboard = new QWaylandClipboard(mDisplay);
#endif
#ifndef QT_NO_DRAGANDDROP
    mDrag = new QWaylandDrag(mDisplay);
#endif
    QString icStr = QPlatformInputContextFactory::requested();
    icStr.isNull() ? mInputContext.reset(new QWaylandInputContext(mDisplay))
                   : mInputContext.reset(QPlatformInputContextFactory::create(icStr));
}

QWaylandIntegration::~QWaylandIntegration()
{
#ifndef QT_NO_DRAGANDDROP
    delete mDrag;
#endif
#ifndef QT_NO_CLIPBOARD
    delete mClipboard;
#endif
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

#ifndef QT_NO_CLIPBOARD
QPlatformClipboard *QWaylandIntegration::clipboard() const
{
    return mClipboard;
}
#endif

#ifndef QT_NO_DRAGANDDROP
QPlatformDrag *QWaylandIntegration::drag() const
{
    return mDrag;
}
#endif

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

#ifndef QT_NO_ACCESSIBILITY
QPlatformAccessibility *QWaylandIntegration::accessibility() const
{
    return mAccessibility;
}
#endif

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

    if (!targetKey.isEmpty()) {
        QStringList keys = QWaylandShellIntegrationFactory::keys();
        if (keys.contains(targetKey)) {
            qDebug("Using the '%s' shell integration", qPrintable(targetKey));
            mShellIntegration = QWaylandShellIntegrationFactory::create(targetKey, QStringList());
        }
    } else {
        QStringList preferredShells;
        if (qEnvironmentVariableIsSet("QT_WAYLAND_USE_XDG_SHELL"))
            preferredShells << QLatin1String("xdg_shell");
        preferredShells << QLatin1String("wl_shell");

        Q_FOREACH (QString preferredShell, preferredShells) {
            if (mDisplay->hasRegistryGlobal(preferredShell)) {
                mShellIntegration = createShellIntegration(preferredShell);
                break;
            }
        }
    }

    Q_ASSERT(mShellIntegration);

    if (!mShellIntegration->initialize(mDisplay)) {
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

QWaylandShellIntegration *QWaylandIntegration::createShellIntegration(const QString &interfaceName)
{
    if (interfaceName == QLatin1Literal("wl_shell")) {
        return new QWaylandWlShellIntegration(mDisplay);
    } else if (interfaceName == QLatin1Literal("xdg_shell")) {
        return new QWaylandXdgShellIntegration(mDisplay);
    } else {
        return Q_NULLPTR;
    }
}

}

QT_END_NAMESPACE
