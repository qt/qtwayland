// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DRMEGLSERVERBUFFERINTEGRATION_H
#define DRMEGLSERVERBUFFERINTEGRATION_H

#include <QtCore/QVariant>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-drm-egl-server-buffer.h"

#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/private/qwayland-server-server-buffer-extension.h>

#include <QtCore/QDebug>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_KHR_image
typedef void *EGLImageKHR;
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
#endif
#ifndef EGL_MESA_drm_image
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEDRMIMAGEMESAPROC) (EGLDisplay dpy, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDRMIMAGEMESAPROC) (EGLDisplay dpy, EGLImageKHR image, EGLint *name, EGLint *handle, EGLint *stride);
#endif

QT_BEGIN_NAMESPACE

class DrmEglServerBufferIntegration;
class QImage;

class DrmEglServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    DrmEglServerBuffer(DrmEglServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;

private:
    DrmEglServerBufferIntegration *m_integration = nullptr;

    EGLImageKHR m_image;

    int32_t m_name;
    int32_t m_stride;
    QOpenGLTexture *m_texture = nullptr;
    QtWaylandServer::qt_drm_egl_server_buffer::format m_drm_format;
};

class DrmEglServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_drm_egl_server_buffer
{
public:
    DrmEglServerBufferIntegration();
    ~DrmEglServerBufferIntegration() override;

    bool initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;

    EGLDisplay display() const { return m_egl_display; }

    inline EGLImageKHR eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    inline EGLBoolean eglDestroyImageKHR (EGLImageKHR image);
    inline EGLImageKHR eglCreateDRMImageMESA (const EGLint *attrib_list);
    inline EGLBoolean eglExportDRMImageMESA (EGLImageKHR image, EGLint *name, EGLint *handle, EGLint *stride);
    inline void glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image);

private:
    EGLDisplay m_egl_display = EGL_NO_DISPLAY;
    PFNEGLCREATEDRMIMAGEMESAPROC m_egl_create_drm_image;
    PFNEGLEXPORTDRMIMAGEMESAPROC m_egl_export_drm_image;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_gl_egl_image_target_texture_2d;

    PFNEGLCREATEIMAGEKHRPROC m_egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC m_egl_destroy_image;
};

EGLImageKHR DrmEglServerBufferIntegration::eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!m_egl_create_image) {
        qWarning("DrmEglServerBufferIntegration: Trying to used unresolved function eglCreateImageKHR");
        return EGL_NO_IMAGE_KHR;
    }
    return m_egl_create_image(m_egl_display, ctx, target, buffer,attrib_list);
}

EGLBoolean DrmEglServerBufferIntegration::eglDestroyImageKHR (EGLImageKHR image)
{
    if (!m_egl_destroy_image) {
        qWarning("DrmEglServerBufferIntegration: Trying to use unresolved function eglDestroyImageKHR");
        return false;
    }
    return m_egl_destroy_image(m_egl_display, image);
}

EGLImageKHR DrmEglServerBufferIntegration::eglCreateDRMImageMESA (const EGLint *attrib_list)
{
    if (m_egl_create_drm_image)
        return m_egl_create_drm_image(m_egl_display, attrib_list);
    else
        qWarning("DrmEglServerBufferIntegration: Trying to use unresolved function eglCreateDRMImageMESA");
    return EGL_NO_IMAGE_KHR;

}

EGLBoolean DrmEglServerBufferIntegration::eglExportDRMImageMESA (EGLImageKHR image, EGLint *name, EGLint *handle, EGLint *stride)
{
    if (m_egl_export_drm_image)
        return m_egl_export_drm_image(m_egl_display, image, name, handle, stride);
    else
        qWarning("DrmEglServerBufferIntegration: Trying to use unresolved function eglExportDRMImageMESA");
    return 0;
}

void DrmEglServerBufferIntegration::glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
{
    if (m_gl_egl_image_target_texture_2d)
        return m_gl_egl_image_target_texture_2d(target, image);
    else
        qWarning("DrmEglServerBufferIntegration: Trying to use unresolved function glEGLImageTargetTexture2DOES");
}
QT_END_NAMESPACE

#endif
