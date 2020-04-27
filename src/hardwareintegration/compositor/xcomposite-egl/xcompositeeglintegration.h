/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef XCOMPOSITEEGLINTEGRATION_H
#define XCOMPOSITEEGLINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/private/qwlclientbuffer_p.h>
#include "xlibinclude.h"

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

class XCompositeEglClientBufferIntegration : public QtWayland::ClientBufferIntegration
{
public:
    XCompositeEglClientBufferIntegration();

    void initializeHardware(struct ::wl_display *display) override;
    QtWayland::ClientBuffer *createBufferFor(wl_resource *buffer) override;
    inline Display *xDisplay() const { return mDisplay; }
    inline EGLDisplay eglDisplay() const { return mEglDisplay; }

private:
    Display *mDisplay = nullptr;
    EGLDisplay mEglDisplay = EGL_NO_DISPLAY;
};

class XCompositeEglClientBuffer : public QtWayland::ClientBuffer
{
public:
    XCompositeEglClientBuffer(XCompositeEglClientBufferIntegration *integration, wl_resource *bufferResource);

    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override {
        return QWaylandBufferRef::BufferFormatEgl_RGBA;
    }

private:
    QOpenGLTexture *m_texture = nullptr;
    XCompositeEglClientBufferIntegration *m_integration = nullptr;
};

QT_END_NAMESPACE

#endif // XCOMPOSITEEGLINTEGRATION_H
