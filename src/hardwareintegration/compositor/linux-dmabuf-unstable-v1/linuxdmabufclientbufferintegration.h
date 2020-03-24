/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef LINUXDMABUFCLIENTBUFFERINTEGRATION_H
#define LINUXDMABUFCLIENTBUFFERINTEGRATION_H

#include "linuxdmabuf.h"

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QtWaylandCompositor/private/qwlclientbuffer_p.h>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>

#include <drm_fourcc.h>

QT_BEGIN_NAMESPACE

typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL_compat) (EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDMABUFFORMATSEXTPROC) (EGLDisplay dpy, EGLint max_formats, EGLint *formats, EGLint *num_formats);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDMABUFMODIFIERSEXTPROC) (EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR *modifiers, EGLBoolean *external_only, EGLint *num_modifiers);

class LinuxDmabufClientBufferIntegrationPrivate;
class LinuxDmabufParams;
class LinuxDmabufClientBuffer;

// buffer conversion definitions to import YUV buffers
struct YuvPlaneConversion {
    EGLint format = DRM_FORMAT_YUYV;
    EGLint widthDivisor = 1;
    EGLint heightDivisor = 1;
    EGLint planeIndex = 0;
};
struct YuvFormatConversion {
    uint32_t inputPlanes = 1;
    uint32_t outputPlanes = 1;
    struct YuvPlaneConversion plane[LinuxDmabufWlBuffer::MaxDmabufPlanes];
};

class LinuxDmabufClientBufferIntegration : public QtWayland::ClientBufferIntegration
{
public:
    LinuxDmabufClientBufferIntegration();
    ~LinuxDmabufClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;
    QtWayland::ClientBuffer *createBufferFor(wl_resource *resource) override;
    bool importBuffer(wl_resource *resource, LinuxDmabufWlBuffer *linuxDmabufBuffer);
    void removeBuffer(wl_resource *resource);
    void deleteOrphanedTextures();
    void deleteImage(EGLImageKHR image);
    void deleteGLTextureWhenPossible(QOpenGLTexture *texture) { m_orphanedTextures << texture; }
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d = nullptr;

private:
    Q_DISABLE_COPY(LinuxDmabufClientBufferIntegration)

    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display = nullptr;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display = nullptr;
    PFNEGLCREATEIMAGEKHRPROC egl_create_image = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image = nullptr;
    PFNEGLQUERYDMABUFMODIFIERSEXTPROC egl_query_dmabuf_modifiers_ext = nullptr;
    PFNEGLQUERYDMABUFFORMATSEXTPROC egl_query_dmabuf_formats_ext = nullptr;

    bool initSimpleTexture(LinuxDmabufWlBuffer *dmabufBuffer);
    bool initYuvTexture(LinuxDmabufWlBuffer *dmabufBuffer);
    QVector<uint32_t> supportedDrmFormats();
    QVector<uint64_t> supportedDrmModifiers(uint32_t format);

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    bool m_displayBound = false;
    QVector<QOpenGLTexture *> m_orphanedTextures;
    QHash<EGLint, YuvFormatConversion> m_yuvFormats;
    bool m_supportsDmabufModifiers = false;
    QHash<struct ::wl_resource *, LinuxDmabufWlBuffer *> m_importedBuffers;
    QScopedPointer<LinuxDmabuf> m_linuxDmabuf;
};

class LinuxDmabufClientBuffer : public QtWayland::ClientBuffer
{
public:
    ~LinuxDmabufClientBuffer() override;

    QWaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    QWaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;

protected:
    void setDestroyed() override;

private:
    friend class LinuxDmabufClientBufferIntegration;
    friend class LinuxDmabufClientBufferIntegrationPrivate;

    LinuxDmabufClientBuffer(LinuxDmabufClientBufferIntegration* integration, wl_resource *bufferResource, LinuxDmabufWlBuffer *dmabufBuffer);

    LinuxDmabufWlBuffer *d = nullptr;
    LinuxDmabufClientBufferIntegration *m_integration = nullptr;
};

QT_END_NAMESPACE

#endif // LINUXDMABUFCLIENTBUFFERINTEGRATION_H
