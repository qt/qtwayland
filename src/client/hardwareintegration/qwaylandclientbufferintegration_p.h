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

#ifndef QWAYLANDCLIENTBUFFERINTEGRATION_H
#define QWAYLANDCLIENTBUFFERINTEGRATION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QtWaylandClient/private/qwaylandclientexport_p.h>

QT_BEGIN_NAMESPACE

class QWindow;
class QPlatformOpenGLContext;
class QSurfaceFormat;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientBufferIntegration
{
public:
    QWaylandClientBufferIntegration();
    virtual ~QWaylandClientBufferIntegration();

    virtual void initialize(QWaylandDisplay *display) = 0;

    virtual bool isValid() const { return true; }

    virtual bool supportsThreadedOpenGL() const { return false; }
    virtual bool supportsWindowDecoration() const { return false; }

    virtual QWaylandWindow *createEglWindow(QWindow *window) = 0;
    virtual QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const = 0;

    enum NativeResource {
        EglDisplay,
        EglConfig,
        EglContext
    };
    virtual void *nativeResource(NativeResource /*resource*/) { return Q_NULLPTR; }
    virtual void *nativeResourceForContext(NativeResource /*resource*/, QPlatformOpenGLContext */*context*/) { return Q_NULLPTR; }
};

}

QT_END_NAMESPACE

#endif // QWAYLANDCLIENTBUFFERINTEGRATION_H
