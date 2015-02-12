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

#ifndef QWAYLANDXCOMPOSITEGLXINTEGRATION_H
#define QWAYLANDXCOMPOSITEGLXINTEGRATION_H

#include <QtWaylandClient/private/qwaylandclientbufferintegration_p.h>
#include <wayland-client.h>

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWindow>

#include <X11/Xlib.h>

// avoid clashes with Qt::CursorShape
#ifdef CursorShape
#   define X_CursorShape CursorShape
#   undef CursorShape
#endif

struct qt_xcomposite;
struct qt_xcomposite_listener;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandXCompositeGLXIntegration : public QWaylandClientBufferIntegration
{
public:
    QWaylandXCompositeGLXIntegration();
    ~QWaylandXCompositeGLXIntegration();

    void initialize(QWaylandDisplay *display) Q_DECL_OVERRIDE;

    QWaylandWindow *createEglWindow(QWindow *window) Q_DECL_OVERRIDE;
    QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const Q_DECL_OVERRIDE;

    QWaylandDisplay *waylandDisplay() const;
    struct qt_xcomposite *waylandXComposite() const;

    Display *xDisplay() const;
    int screen() const;
    Window rootWindow() const;

    bool supportsThreadedOpenGL() const { return false; }
    bool supportsWindowDecoration() const { return false; }

private:
    QWaylandDisplay *mWaylandDisplay;
    struct qt_xcomposite *mWaylandComposite;

    Display *mDisplay;
    int mScreen;
    Window mRootWindow;

    static void wlDisplayHandleGlobal(void *data, struct wl_registry *registry, uint32_t id,
                                      const QString &interface, uint32_t version);

    static const struct qt_xcomposite_listener xcomposite_listener;
    static void rootInformation(void *data,
                 struct qt_xcomposite *xcomposite,
                 const char *display_name,
                 uint32_t root_window);
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXCOMPOSITEGLXINTEGRATION_H
