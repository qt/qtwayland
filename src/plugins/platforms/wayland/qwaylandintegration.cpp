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

#include "qwaylandintegration.h"

#include "qwaylanddisplay.h"
#include "qwaylandshmbackingstore.h"
#include "qwaylandshmwindow.h"
#include "qwaylandnativeinterface.h"
#include "qwaylandclipboard.h"
#include "qwaylanddnd.h"

#include "QtPlatformSupport/private/qgenericunixfontdatabase_p.h"
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

#include <QtGui/private/qguiapplication_p.h>

#include <QtGui/QWindowSystemInterface>
#include <qpa/qplatformcursor.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>

#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformaccessibility.h>
#include <qpa/qplatforminputcontext.h>

#ifdef QT_WAYLAND_GL_SUPPORT
#include "gl_integration/qwaylandglintegration.h"
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

QWaylandIntegration::QWaylandIntegration()
    : mFontDb(new QGenericUnixFontDatabase())
    , mEventDispatcher(createUnixEventDispatcher())
    , mNativeInterface(new QWaylandNativeInterface(this))
    , mAccessibility(new QPlatformAccessibility())
{
    QGuiApplicationPrivate::instance()->setEventDispatcher(mEventDispatcher);
    mDisplay = new QWaylandDisplay();
    mClipboard = new QWaylandClipboard(mDisplay);
    mDrag = new QWaylandDrag(mDisplay);

    foreach (QPlatformScreen *screen, mDisplay->screens())
        screenAdded(screen);

    mInputContext = QPlatformInputContextFactory::create();
}

QWaylandIntegration::~QWaylandIntegration()
{
    delete mDrag;
    delete mClipboard;
    delete mAccessibility;
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
#ifdef QT_WAYLAND_GL_SUPPORT
        return true;
#else
        return false;
#endif
    case ThreadedOpenGL:
#ifdef QT_WAYLAND_GL_SUPPORT
        return mDisplay->eglIntegration()->supportsThreadedOpenGL();
#else
        return false;
#endif
    case BufferQueueingOpenGL:
        return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
#ifdef QT_WAYLAND_GL_SUPPORT
    if (window->surfaceType() == QWindow::OpenGLSurface)
        return mDisplay->eglIntegration()->createEglWindow(window);
#endif
    return new QWaylandShmWindow(window);
}

QPlatformOpenGLContext *QWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
#ifdef QT_WAYLAND_GL_SUPPORT
    return mDisplay->eglIntegration()->createPlatformOpenGLContext(context->format(), context->shareHandle());
#else
    Q_UNUSED(context);
    return 0;
#endif
}

QPlatformBackingStore *QWaylandIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QWaylandShmBackingStore(window);
}

QAbstractEventDispatcher *QWaylandIntegration::guiThreadEventDispatcher() const
{
    return mEventDispatcher;
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
    return mInputContext;
}

QVariant QWaylandIntegration::styleHint(StyleHint hint) const
{
#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    if (hint == ShowIsFullScreen && mDisplay->windowManagerIntegration())
        return mDisplay->windowManagerIntegration()->showIsFullScreen();
#endif
    return QPlatformIntegration::styleHint(hint);
}

QPlatformAccessibility *QWaylandIntegration::accessibility() const
{
    return mAccessibility;
}

QPlatformServices *QWaylandIntegration::services() const
{
#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    return mDisplay->windowManagerIntegration();
#else
    return QWaylandIntegration::services();
#endif
}

QWaylandDisplay *QWaylandIntegration::display() const
{
    return mDisplay;
}

QStringList QWaylandIntegration::themeNames() const
{
    return QGenericUnixTheme::themeNames();
}

QPlatformTheme *QWaylandIntegration::createPlatformTheme(const QString &name) const
{
    return QGenericUnixTheme::createUnixTheme(name);
}
