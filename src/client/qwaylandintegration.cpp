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
#if QT_CONFIG(clipboard)
#include "qwaylandclipboard_p.h"
#endif
#include "qwaylanddnd_p.h"
#include "qwaylandwindowmanagerintegration_p.h"
#include "qwaylandscreen_p.h"

#if defined(Q_OS_MACOS)
#  include <QtFontDatabaseSupport/private/qcoretextfontdatabase_p.h>
#  include <QtFontDatabaseSupport/private/qfontengine_coretext_p.h>
#else
#  include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#endif
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtThemeSupport/private/qgenericunixthemes_p.h>

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

#if QT_CONFIG(accessibility_atspi_bridge)
#include <QtLinuxAccessibilitySupport/private/bridge_p.h>
#endif

#if QT_CONFIG(xkbcommon)
#include <QtXkbCommonSupport/private/qxkbcommon_p.h>
#endif

#if QT_CONFIG(vulkan)
#include "qwaylandvulkaninstance_p.h"
#include "qwaylandvulkanwindow_p.h"
#endif

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandIntegration::QWaylandIntegration()
#if defined(Q_OS_MACOS)
    : mFontDb(new QCoreTextFontDatabaseEngineFactory<QCoreTextFontEngine>)
#else
    : mFontDb(new QGenericUnixFontDatabase())
#endif
    , mNativeInterface(new QWaylandNativeInterface(this))
{
    initializeInputDeviceIntegration();
    mDisplay.reset(new QWaylandDisplay(this));
    if (!mDisplay->isInitialized()) {
        mFailed = true;
        return;
    }
#if QT_CONFIG(clipboard)
    mClipboard.reset(new QWaylandClipboard(mDisplay.data()));
#endif
#if QT_CONFIG(draganddrop)
    mDrag.reset(new QWaylandDrag(mDisplay.data()));
#endif

    reconfigureInputContext();
}

QWaylandIntegration::~QWaylandIntegration()
{
}

QPlatformNativeInterface * QWaylandIntegration::nativeInterface() const
{
    return mNativeInterface.data();
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
    case WindowActivation:
        return false;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
    if ((window->surfaceType() == QWindow::OpenGLSurface || window->surfaceType() == QWindow::RasterGLSurface)
        && mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createEglWindow(window);

#if QT_CONFIG(vulkan)
    if (window->surfaceType() == QSurface::VulkanSurface)
        return new QWaylandVulkanWindow(window, mDisplay.data());
#endif // QT_CONFIG(vulkan)

    return new QWaylandShmWindow(window, mDisplay.data());
}

#if QT_CONFIG(opengl)
QPlatformOpenGLContext *QWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    if (mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createPlatformOpenGLContext(context->format(), context->shareHandle());
    return nullptr;
}
#endif  // opengl

QPlatformBackingStore *QWaylandIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QWaylandShmBackingStore(window, mDisplay.data());
}

QAbstractEventDispatcher *QWaylandIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

void QWaylandIntegration::initialize()
{
    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    QObject::connect(dispatcher, SIGNAL(aboutToBlock()), mDisplay.data(), SLOT(flushRequests()));
    QObject::connect(dispatcher, SIGNAL(awake()), mDisplay.data(), SLOT(flushRequests()));

    int fd = wl_display_get_fd(mDisplay->wl_display());
    QSocketNotifier *sn = new QSocketNotifier(fd, QSocketNotifier::Read, mDisplay.data());
    QObject::connect(sn, SIGNAL(activated(QSocketDescriptor)), mDisplay.data(), SLOT(flushRequests()));

    // Qt does not support running with no screens
    mDisplay->ensureScreen();
}

QPlatformFontDatabase *QWaylandIntegration::fontDatabase() const
{
    return mFontDb.data();
}

#if QT_CONFIG(clipboard)
QPlatformClipboard *QWaylandIntegration::clipboard() const
{
    return mClipboard.data();
}
#endif

#if QT_CONFIG(draganddrop)
QPlatformDrag *QWaylandIntegration::drag() const
{
    return mDrag.data();
}
#endif  // draganddrop

QPlatformInputContext *QWaylandIntegration::inputContext() const
{
    return mInputContext.data();
}

QVariant QWaylandIntegration::styleHint(StyleHint hint) const
{
    if (hint == ShowIsFullScreen && mDisplay->windowManagerIntegration())
        return mDisplay->windowManagerIntegration()->showIsFullScreen();

    return QPlatformIntegration::styleHint(hint);
}

#if QT_CONFIG(accessibility)
QPlatformAccessibility *QWaylandIntegration::accessibility() const
{
    if (!mAccessibility) {
#if QT_CONFIG(accessibility_atspi_bridge)
        Q_ASSERT_X(QCoreApplication::eventDispatcher(), "QWaylandIntegration",
            "Initializing accessibility without event-dispatcher!");
        mAccessibility.reset(new QSpiAccessibleBridge());
#else
        mAccessibility.reset(new QPlatformAccessibility());
#endif
    }
    return mAccessibility.data();
}
#endif

QPlatformServices *QWaylandIntegration::services() const
{
    return mDisplay->windowManagerIntegration();
}

QWaylandDisplay *QWaylandIntegration::display() const
{
    return mDisplay.data();
}

QList<int> QWaylandIntegration::possibleKeys(const QKeyEvent *event) const
{
    if (auto *seat = mDisplay->currentInputDevice())
        return seat->possibleKeys(event);
    return {};
}

QStringList QWaylandIntegration::themeNames() const
{
    return QGenericUnixTheme::themeNames();
}

QPlatformTheme *QWaylandIntegration::createPlatformTheme(const QString &name) const
{
    return QGenericUnixTheme::createUnixTheme(name);
}

#if QT_CONFIG(vulkan)
QPlatformVulkanInstance *QWaylandIntegration::createPlatformVulkanInstance(QVulkanInstance *instance) const
{
    return new QWaylandVulkanInstance(instance);
}
#endif // QT_CONFIG(vulkan)

// May be called from non-GUI threads
QWaylandClientBufferIntegration *QWaylandIntegration::clientBufferIntegration() const
{
    // Do an inexpensive check first to avoid locking whenever possible
    if (Q_UNLIKELY(!mClientBufferIntegrationInitialized))
        const_cast<QWaylandIntegration *>(this)->initializeClientBufferIntegration();

    Q_ASSERT(mClientBufferIntegrationInitialized);
    return mClientBufferIntegration && mClientBufferIntegration->isValid() ? mClientBufferIntegration.data() : nullptr;
}

QWaylandServerBufferIntegration *QWaylandIntegration::serverBufferIntegration() const
{
    if (!mServerBufferIntegrationInitialized)
        const_cast<QWaylandIntegration *>(this)->initializeServerBufferIntegration();

    return mServerBufferIntegration.data();
}

QWaylandShellIntegration *QWaylandIntegration::shellIntegration() const
{
    if (!mShellIntegrationInitialized)
        const_cast<QWaylandIntegration *>(this)->initializeShellIntegration();

    return mShellIntegration.data();
}

// May be called from non-GUI threads
void QWaylandIntegration::initializeClientBufferIntegration()
{
    QMutexLocker lock(&mClientBufferInitLock);
    if (mClientBufferIntegrationInitialized)
        return;

    QString targetKey = QString::fromLocal8Bit(qgetenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION"));

    if (targetKey.isEmpty()) {
        if (mDisplay->hardwareIntegration()
                && mDisplay->hardwareIntegration()->clientBufferIntegration() != QLatin1String("wayland-eglstream-controller")
                && mDisplay->hardwareIntegration()->clientBufferIntegration() != QLatin1String("linux-dmabuf-unstable-v1")) {
            targetKey = mDisplay->hardwareIntegration()->clientBufferIntegration();
        } else {
            targetKey = QLatin1String("wayland-egl");
        }
    }

    if (targetKey.isEmpty()) {
        qWarning("Failed to determine what client buffer integration to use");
    } else {
        QStringList keys = QWaylandClientBufferIntegrationFactory::keys();
        qCDebug(lcQpaWayland) << "Available client buffer integrations:" << keys;

        if (keys.contains(targetKey))
            mClientBufferIntegration.reset(QWaylandClientBufferIntegrationFactory::create(targetKey, QStringList()));

        if (mClientBufferIntegration) {
            qCDebug(lcQpaWayland) << "Initializing client buffer integration" << targetKey;
            mClientBufferIntegration->initialize(mDisplay.data());
        } else {
            qCWarning(lcQpaWayland) << "Failed to load client buffer integration:" << targetKey;
            qCWarning(lcQpaWayland) << "Available client buffer integrations:" << keys;
        }
    }

    // This must be set last to make sure other threads don't use the
    // integration before initialization is complete.
    mClientBufferIntegrationInitialized = true;
}

void QWaylandIntegration::initializeServerBufferIntegration()
{
    mServerBufferIntegrationInitialized = true;

    QString targetKey = QString::fromLocal8Bit(qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION"));

    if (targetKey.isEmpty() && mDisplay->hardwareIntegration())
        targetKey = mDisplay->hardwareIntegration()->serverBufferIntegration();

    if (targetKey.isEmpty()) {
        qWarning("Failed to determine what server buffer integration to use");
        return;
    }

    QStringList keys = QWaylandServerBufferIntegrationFactory::keys();
    qCDebug(lcQpaWayland) << "Available server buffer integrations:" << keys;

    if (keys.contains(targetKey))
        mServerBufferIntegration.reset(QWaylandServerBufferIntegrationFactory::create(targetKey, QStringList()));

    if (mServerBufferIntegration) {
        qCDebug(lcQpaWayland) << "Initializing server buffer integration" << targetKey;
        mServerBufferIntegration->initialize(mDisplay.data());
    } else {
        qCWarning(lcQpaWayland) << "Failed to load server buffer integration: " <<  targetKey;
        qCWarning(lcQpaWayland) << "Available server buffer integrations:" << keys;
    }
}

void QWaylandIntegration::initializeShellIntegration()
{
    mShellIntegrationInitialized = true;

    QByteArray integrationNames = qgetenv("QT_WAYLAND_SHELL_INTEGRATION");
    QString targetKeys = QString::fromLocal8Bit(integrationNames);

    QStringList preferredShells;
    if (!targetKeys.isEmpty()) {
        preferredShells = targetKeys.split(QLatin1Char(';'));
    } else {
        preferredShells << QLatin1String("xdg-shell");
        preferredShells << QLatin1String("xdg-shell-v6");
        QString useXdgShell = QString::fromLocal8Bit(qgetenv("QT_WAYLAND_USE_XDG_SHELL"));
        if (!useXdgShell.isEmpty() && useXdgShell != QLatin1String("0")) {
            qWarning() << "QT_WAYLAND_USE_XDG_SHELL is deprecated, "
                          "please specify the shell using QT_WAYLAND_SHELL_INTEGRATION instead";
            preferredShells << QLatin1String("xdg-shell-v5");
        }
        preferredShells << QLatin1String("wl-shell") << QLatin1String("ivi-shell");
    }

    for (const QString &preferredShell : qAsConst(preferredShells)) {
        mShellIntegration.reset(createShellIntegration(preferredShell));
        if (mShellIntegration) {
            qCDebug(lcQpaWayland, "Using the '%s' shell integration", qPrintable(preferredShell));
            break;
        }
    }

    if (!mShellIntegration) {
        qCWarning(lcQpaWayland) << "Loading shell integration failed.";
        qCWarning(lcQpaWayland) << "Attempted to load the following shells" << preferredShells;
    }

    QWindowSystemInterfacePrivate::TabletEvent::setPlatformSynthesizesMouse(false);
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
        mInputDeviceIntegration.reset(QWaylandInputDeviceIntegrationFactory::create(targetKey, QStringList()));
        qDebug("Using the '%s' input device integration", qPrintable(targetKey));
    } else {
        qWarning("Wayland inputdevice integration '%s' not found, using default", qPrintable(targetKey));
    }
}

void QWaylandIntegration::reconfigureInputContext()
{
    if (!mDisplay) {
        // This function can be called from QWaylandDisplay::registry_global() when we
        // are in process of constructing QWaylandDisplay. Configuring input context
        // in that case is done by calling reconfigureInputContext() from QWaylandIntegration
        // constructor, after QWaylandDisplay has been constructed.
        return;
    }

    const QString &requested = QPlatformInputContextFactory::requested();
    if (requested == QLatin1String("qtvirtualkeyboard"))
        qCWarning(lcQpaWayland) << "qtvirtualkeyboard currently is not supported at client-side,"
                                   " use QT_IM_MODULE=qtvirtualkeyboard at compositor-side.";

    if (requested.isNull())
        mInputContext.reset(new QWaylandInputContext(mDisplay.data()));
    else
        mInputContext.reset(QPlatformInputContextFactory::create(requested));

    const QString defaultInputContext(QStringLiteral("compose"));
    if ((!mInputContext || !mInputContext->isValid()) && requested != defaultInputContext)
        mInputContext.reset(QPlatformInputContextFactory::create(defaultInputContext));

#if QT_CONFIG(xkbcommon)
    QXkbCommon::setXkbContext(mInputContext.data(), mDisplay->xkbContext());
#endif

    // Even if compositor-side input context handling has been requested, we fallback to
    // client-side handling if compositor does not provide the text-input extension. This
    // is why we need to check here which input context actually is being used.
    mDisplay->mUsingInputContextFromCompositor = qobject_cast<QWaylandInputContext *>(mInputContext.data());

    qCDebug(lcQpaWayland) << "using input method:" << inputContext()->metaObject()->className();
}

QWaylandShellIntegration *QWaylandIntegration::createShellIntegration(const QString &integrationName)
{
    if (QWaylandShellIntegrationFactory::keys().contains(integrationName)) {
        return QWaylandShellIntegrationFactory::create(integrationName, mDisplay.data());
    } else {
        qCWarning(lcQpaWayland) << "No shell integration named" << integrationName << "found";
        return nullptr;
    }
}

}

QT_END_NAMESPACE
