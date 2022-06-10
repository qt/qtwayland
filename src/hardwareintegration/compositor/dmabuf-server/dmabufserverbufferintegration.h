// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DMABUFSERVERBUFFERINTEGRATION_H
#define DMABUFSERVERBUFFERINTEGRATION_H

#include <QtCore/QVariant>
#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-qt-dmabuf-server-buffer.h"

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

#ifndef EGL_MESA_image_dma_buf_export
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) (EGLDisplay dpy, EGLImageKHR image, int *fourcc, int *num_planes, EGLuint64KHR *modifiers);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDMABUFIMAGEMESAPROC) (EGLDisplay dpy, EGLImageKHR image, int *fds, EGLint *strides, EGLint *offsets);
#endif

QT_BEGIN_NAMESPACE

class DmaBufServerBufferIntegration;
class QImage;

class DmaBufServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_server_buffer
{
public:
    DmaBufServerBuffer(DmaBufServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);
    ~DmaBufServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    QOpenGLTexture *toOpenGlTexture() override;
    bool bufferInUse() override;

protected:
    void server_buffer_release(Resource *resource) override;

private:
    DmaBufServerBufferIntegration *m_integration = nullptr;

    EGLImageKHR m_image;

    int32_t m_offset;
    int32_t m_stride;
    QOpenGLTexture *m_texture = nullptr;
    int m_fourcc_format;
    int m_fd;
};

class DmaBufServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_dmabuf_server_buffer
{
public:
    DmaBufServerBufferIntegration();
    ~DmaBufServerBufferIntegration() override;

    bool initializeHardware(QWaylandCompositor *) override;

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;

    EGLDisplay display() const { return m_egl_display; }

    inline EGLImageKHR eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    inline EGLBoolean eglDestroyImageKHR(EGLImageKHR image);

    inline EGLBoolean eglExportDMABUFImageQueryMESA(EGLImageKHR image,
                                                    int *fourcc,
                                                    int *num_planes,
                                                    EGLuint64KHR *modifiers);

    inline EGLBoolean eglExportDMABUFImageMESA(EGLImageKHR image,
                                               int *fds,
                                               EGLint *strides,
                                               EGLint *offsets);

    inline void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);

private:
    EGLDisplay m_egl_display;

    PFNEGLEXPORTDMABUFIMAGEMESAPROC m_egl_export_dmabuf_image = nullptr;
    PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC m_egl_export_dmabuf_image_query = nullptr;

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_gl_egl_image_target_texture_2d = nullptr;
    PFNEGLCREATEIMAGEKHRPROC m_egl_create_image = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC m_egl_destroy_image = nullptr;
};

EGLImageKHR DmaBufServerBufferIntegration::eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!m_egl_create_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglCreateImageKHR";
        return EGL_NO_IMAGE_KHR;
    }
    return m_egl_create_image(m_egl_display, ctx, target, buffer, attrib_list);
}

EGLBoolean DmaBufServerBufferIntegration::eglDestroyImageKHR(EGLImageKHR image)
{
    if (!m_egl_destroy_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglDestroyImageKHR";
        return false;
    }
    return m_egl_destroy_image(m_egl_display, image);
}

EGLBoolean DmaBufServerBufferIntegration::eglExportDMABUFImageQueryMESA(EGLImageKHR image,
                                                                        int *fourcc,
                                                                        int *num_planes,
                                                                        EGLuint64KHR *modifiers)
{
    if (m_egl_export_dmabuf_image_query)
        return m_egl_export_dmabuf_image_query(m_egl_display, image, fourcc, num_planes, modifiers);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglExportDMABUFImageQueryMESA";
    return false;
}

EGLBoolean DmaBufServerBufferIntegration::eglExportDMABUFImageMESA(EGLImageKHR image,
                                                                   int *fds,
                                                                   EGLint *strides,
                                                                   EGLint *offsets)
{
    if (m_egl_export_dmabuf_image)
        return m_egl_export_dmabuf_image(m_egl_display, image, fds, strides, offsets);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function eglExportDMABUFImageMESA";
    return false;
}

void DmaBufServerBufferIntegration::glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    if (m_gl_egl_image_target_texture_2d)
        return m_gl_egl_image_target_texture_2d(target, image);
    else
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "DmaBufServerBufferIntegration: Trying to use unresolved function glEGLImageTargetTexture2DOES";
}
QT_END_NAMESPACE

#endif
