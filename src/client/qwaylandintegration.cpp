/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandintegration.h"

#include "qwaylanddisplay.h"
#include "qwaylandinputcontext.h"
#include "qwaylandshmbackingstore.h"
#include "qwaylandshmwindow.h"
#include "qwaylandnativeinterface.h"
#include "qwaylandclipboard.h"
#include "qwaylanddnd.h"
#include "qwaylandwindowmanagerintegration.h"

#include "QtPlatformSupport/private/qgenericunixfontdatabase_p.h"
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformcursor.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>

#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformaccessibility.h>
#include <qpa/qplatforminputcontext.h>

#include "qwaylandclientbufferintegration.h"
#include "qwaylandclientbufferintegrationfactory.h"


QT_BEGIN_NAMESPACE

class GenericWaylandTheme: public QGenericUnixTheme
{
public:
    static QStringList themeNames()
    {
        QStringList result;

        if (QGuiApplication::desktopSettingsAware()) {
            const QByteArray desktopEnvironment = QGuiApplicationPrivate::platformIntegration()->services()->desktopEnvironment();

            // Ignore X11 desktop environments
            if (!desktopEnvironment.isEmpty() &&
                desktopEnvironment != QByteArrayLiteral("UNKNOWN") &&
                desktopEnvironment != QByteArrayLiteral("KDE") &&
                desktopEnvironment != QByteArrayLiteral("GNOME") &&
                desktopEnvironment != QByteArrayLiteral("UNITY") &&
                desktopEnvironment != QByteArrayLiteral("MATE") &&
                desktopEnvironment != QByteArrayLiteral("XFCE") &&
                desktopEnvironment != QByteArrayLiteral("LXDE"))
                result.push_back(desktopEnvironment.toLower());
        }

        if (result.isEmpty())
            result.push_back(QLatin1String(QGenericUnixTheme::name));

        return result;
    }
};

QWaylandIntegration::QWaylandIntegration()
    : mClientBufferIntegration(0)
    , mFontDb(new QGenericUnixFontDatabase())
    , mNativeInterface(new QWaylandNativeInterface(this))
#ifndef QT_NO_ACCESSIBILITY
    , mAccessibility(new QPlatformAccessibility())
#else
    , mAccessibility(0)
#endif
    , mClientBufferIntegrationInitialized(false)
{
    mDisplay = new QWaylandDisplay(this);
    mClipboard = new QWaylandClipboard(mDisplay);
    mDrag = new QWaylandDrag(mDisplay);

    foreach (QPlatformScreen *screen, mDisplay->screens())
        screenAdded(screen);

    mInputContext.reset(new QWaylandInputContext(mDisplay));
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
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
    if (window->surfaceType() == QWindow::OpenGLSurface && mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createEglWindow(window);
    return new QWaylandShmWindow(window);
}

QPlatformOpenGLContext *QWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    if (mDisplay->clientBufferIntegration())
        return mDisplay->clientBufferIntegration()->createPlatformOpenGLContext(context->format(), context->shareHandle());
    return 0;
}

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
        const_cast<QWaylandIntegration *>(this)->initializeBufferIntegration();

    return mClientBufferIntegration;
}

void QWaylandIntegration::initializeBufferIntegration()
{
    mClientBufferIntegrationInitialized = true;

    QByteArray clientBufferIntegrationName = qgetenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION");
    if (clientBufferIntegrationName.isEmpty())
        clientBufferIntegrationName = QByteArrayLiteral("wayland-egl");

    QStringList keys = QWaylandClientBufferIntegrationFactory::keys();
    QString targetKey = QString::fromLocal8Bit(clientBufferIntegrationName);
    if (keys.contains(targetKey)) {
        mClientBufferIntegration = QWaylandClientBufferIntegrationFactory::create(targetKey, QStringList());
    }
}

QT_END_NAMESPACE
