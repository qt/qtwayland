/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qwaylandxcompositeglxwindow.h"
#include "qwaylandxcompositebuffer.h"

#include <QtCore/QDebug>

#include "wayland-xcomposite-client-protocol.h"
#include <QtGui/QRegion>

#include <X11/extensions/Xcomposite.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandXCompositeGLXWindow::QWaylandXCompositeGLXWindow(QWindow *window, QWaylandXCompositeGLXIntegration *glxIntegration)
    : QWaylandWindow(window)
    , m_glxIntegration(glxIntegration)
    , m_xWindow(0)
    , m_config(qglx_findConfig(glxIntegration->xDisplay(), glxIntegration->screen(), window->format(), GLX_WINDOW_BIT | GLX_PIXMAP_BIT))
{
}

QWaylandWindow::WindowType QWaylandXCompositeGLXWindow::windowType() const
{
    //yeah. this type needs a new name
    return QWaylandWindow::Egl;
}

void QWaylandXCompositeGLXWindow::setGeometry(const QRect &rect)
{
    QWaylandWindow::setGeometry(rect);

    if (m_xWindow) {
        XDestroyWindow(m_glxIntegration->xDisplay(), m_xWindow);
        m_xWindow = 0;
    }
}

Window QWaylandXCompositeGLXWindow::xWindow() const
{
    if (!m_xWindow)
        const_cast<QWaylandXCompositeGLXWindow *>(this)->createSurface();

    return m_xWindow;
}

void QWaylandXCompositeGLXWindow::createSurface()
{
    QSize size(geometry().size());
    if (size.isEmpty()) {
        //QGLWidget wants a context for a window without geometry
        size = QSize(1,1);
    }

    if (!m_glxIntegration->xDisplay()) {
        qWarning("XCompositeGLXWindow: X display still null?!");
        return;
    }

    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(m_glxIntegration->xDisplay(), m_config);
    Colormap cmap = XCreateColormap(m_glxIntegration->xDisplay(), m_glxIntegration->rootWindow(),
            visualInfo->visual, AllocNone);

    XSetWindowAttributes a;
    a.background_pixel = WhitePixel(m_glxIntegration->xDisplay(), m_glxIntegration->screen());
    a.border_pixel = BlackPixel(m_glxIntegration->xDisplay(), m_glxIntegration->screen());
    a.colormap = cmap;
    m_xWindow = XCreateWindow(m_glxIntegration->xDisplay(), m_glxIntegration->rootWindow(),0, 0, size.width(), size.height(),
                              0, visualInfo->depth, InputOutput, visualInfo->visual,
                              CWBackPixel|CWBorderPixel|CWColormap, &a);

    XCompositeRedirectWindow(m_glxIntegration->xDisplay(), m_xWindow, CompositeRedirectManual);
    XMapWindow(m_glxIntegration->xDisplay(), m_xWindow);

    XSync(m_glxIntegration->xDisplay(), False);
    mBuffer = new QWaylandXCompositeBuffer(m_glxIntegration->waylandXComposite(),
                                            (uint32_t)m_xWindow,
                                            size);
}

}

QT_END_NAMESPACE
