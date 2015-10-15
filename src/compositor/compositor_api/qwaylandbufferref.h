/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QWAYLANDBUFFERREF_H
#define QWAYLANDBUFFERREF_H

#include <QImage>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QtGui/qopengl.h>
#endif

#include <QtCompositor/qwaylandexport.h>

QT_BEGIN_NAMESPACE

namespace QtWayland
{
    class SurfaceBuffer;
}

class Q_COMPOSITOR_EXPORT QWaylandBufferRef
{
public:
    QWaylandBufferRef();
    explicit QWaylandBufferRef(QtWayland::SurfaceBuffer *buffer);
    QWaylandBufferRef(const QWaylandBufferRef &ref);
    ~QWaylandBufferRef();

    QWaylandBufferRef &operator=(const QWaylandBufferRef &ref);
    operator bool() const;
    bool isShm() const;

    QImage image() const;
#ifdef QT_COMPOSITOR_WAYLAND_GL
    /**
     * There must be a GL context bound when calling this function.
     * The texture will be automatically destroyed when the last QWaylandBufferRef
     * referring to the same underlying buffer will be destroyed or reset.
     */
    GLuint createTexture();
    GLenum textureTarget() const;
    void updateTexture();
    void destroyTexture();
    void *nativeBuffer() const;
#endif

private:
    class QWaylandBufferRefPrivate *const d;
    friend class QWaylandBufferRefPrivate;
};

QT_END_NAMESPACE

#endif
