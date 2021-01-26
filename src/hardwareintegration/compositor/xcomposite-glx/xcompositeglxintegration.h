/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef XCOMPOSITEGLXINTEGRATION_H
#define XCOMPOSITEGLXINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QtWaylandCompositor/private/qwlclientbuffer_p.h>
#include "xlibinclude.h"

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

QT_BEGIN_NAMESPACE

class XCompositeHandler;

class XCompositeGLXClientBufferIntegration : public QtWayland::ClientBufferIntegration
{
public:
    XCompositeGLXClientBufferIntegration();
    ~XCompositeGLXClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;
    QtWayland::ClientBuffer *createBufferFor(wl_resource *buffer) override;

    inline Display *xDisplay() const { return mDisplay; }
    inline int xScreen() const { return mScreen; }

    PFNGLXBINDTEXIMAGEEXTPROC m_glxBindTexImageEXT;
    PFNGLXRELEASETEXIMAGEEXTPROC m_glxReleaseTexImageEXT;

private:
    Display *mDisplay = nullptr;
    int mScreen;
    XCompositeHandler *mHandler = nullptr;
};

class XCompositeGLXClientBuffer : public QtWayland::ClientBuffer
{
public:
    XCompositeGLXClientBuffer(XCompositeGLXClientBufferIntegration *integration, wl_resource *bufferResource);

    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override {
        return QWaylandBufferRef::BufferFormatEgl_RGBA;
    }

private:
    QOpenGLTexture *m_texture = nullptr;
    XCompositeGLXClientBufferIntegration *m_integration = nullptr;
    GLXPixmap m_glxPixmap = 0;
};

QT_END_NAMESPACE

#endif // XCOMPOSITEGLXINTEGRATION_H
