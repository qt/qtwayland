// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef LIBHYBRISEGLSERVERBUFFERINTEGRATION_H
#define LIBHYBRISEGLSERVERBUFFERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlserverbufferintegration_p.h>

#include "qwayland-server-libhybris-egl-server-buffer.h"

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

#ifndef EGL_HYBRIS_native_buffer
typedef EGLBoolean (EGLAPIENTRYP PFNEGLHYBRISCREATENATIVEBUFFERPROC)(EGLint width, EGLint height, EGLint usage, EGLint format, EGLint *stride, EGLClientBuffer *buffer);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLHYBRISGETNATIVEBUFFERINFOPROC)(EGLClientBuffer buffer, int *num_ints, int *num_fds);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLHYBRISSERIALIZENATIVEBUFFERPROC)(EGLClientBuffer buffer, int *ints, int *fds);
#endif

#ifndef GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
#endif

QT_BEGIN_NAMESPACE

class LibHybrisEglServerBufferIntegration;

class LibHybrisEglServerBuffer : public QtWayland::ServerBuffer, public QtWaylandServer::qt_libhybris_buffer
{
public:
    LibHybrisEglServerBuffer(LibHybrisEglServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format);

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    QOpenGLTexture *toOpenGlTexture() override;

private:
    LibHybrisEglServerBufferIntegration *m_integration = nullptr;

    EGLImageKHR m_image;
    EGLClientBuffer m_buffer;

    int32_t m_name;
    int32_t m_stride;
    QOpenGLTexture *m_texture = nullptr;
    QtWaylandServer::qt_libhybris_egl_server_buffer::format m_hybris_format;
    QList<int32_t> m_ints;
    QList<int32_t> m_fds;
};

class LibHybrisEglServerBufferIntegration :
    public QtWayland::ServerBufferIntegration,
    public QtWaylandServer::qt_libhybris_egl_server_buffer
{
public:
    LibHybrisEglServerBufferIntegration();
    ~LibHybrisEglServerBufferIntegration();

    bool initializeHardware(QWaylandCompositor *);

    bool supportsFormat(QtWayland::ServerBuffer::Format format) const override;
    QtWayland::ServerBuffer *createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format) override;

    EGLDisplay display() const { return m_egl_display; }

    inline EGLImageKHR eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    inline EGLBoolean eglDestroyImageKHR (EGLImageKHR image);
    inline void glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image);

    inline EGLBoolean eglHybrisCreateNativeBuffer(EGLint width, EGLint height, EGLint usage, EGLint format, EGLint *stride, EGLClientBuffer *buffer);
    inline void eglHybrisGetNativeBufferInfo(EGLClientBuffer buffer, int *num_ints, int *num_fds);
    inline void eglHybrisSerializeNativeBuffer(EGLClientBuffer buffer, int *ints, int *fds);

private:
    EGLDisplay m_egl_display = EGL_NO_DISPLAY;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_gl_egl_image_target_texture_2d;

    PFNEGLHYBRISCREATENATIVEBUFFERPROC m_egl_create_buffer;
    PFNEGLHYBRISGETNATIVEBUFFERINFOPROC m_egl_get_buffer_info;
    PFNEGLHYBRISSERIALIZENATIVEBUFFERPROC m_egl_serialize_buffer;

    PFNEGLCREATEIMAGEKHRPROC m_egl_create_image;
    PFNEGLDESTROYIMAGEKHRPROC m_egl_destroy_image;
};

EGLImageKHR LibHybrisEglServerBufferIntegration::eglCreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!m_egl_create_image) {
        qWarning("LibHybrisEglServerBufferIntegration: Trying to used unresolved function eglCreateImageKHR");
        return EGL_NO_IMAGE_KHR;
    }
    return m_egl_create_image(m_egl_display, ctx, target, buffer,attrib_list);
}

EGLBoolean LibHybrisEglServerBufferIntegration::eglDestroyImageKHR (EGLImageKHR image)
{
    if (!m_egl_destroy_image) {
        qWarning("LibHybrisEglServerBufferIntegration: Trying to use unresolved function eglDestroyImageKHR");
        return false;
    }
    return m_egl_destroy_image(m_egl_display, image);
}

void LibHybrisEglServerBufferIntegration::glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
{
    if (m_gl_egl_image_target_texture_2d)
        return m_gl_egl_image_target_texture_2d(target, image);
    else
        qWarning("LibHybrisEglServerBufferIntegration: Trying to use unresolved function glEGLImageTargetTexture2DOES");
}

EGLBoolean LibHybrisEglServerBufferIntegration::eglHybrisCreateNativeBuffer(EGLint width, EGLint height, EGLint usage, EGLint format, EGLint *stride, EGLClientBuffer *buffer)
{
    if (m_egl_create_buffer)
        return m_egl_create_buffer(width, height, usage, format, stride, buffer);
    else
        qWarning("LibHybrisEglServerBufferIntegration: Trying to use unresolved function eglHybrisCreateNativeBuffer");
    return EGL_FALSE;
}

void LibHybrisEglServerBufferIntegration::eglHybrisGetNativeBufferInfo(EGLClientBuffer buffer, int *num_ints, int *num_fds)
{
    if (m_egl_get_buffer_info)
        m_egl_get_buffer_info(buffer, num_ints, num_fds);
    else
        qWarning("LibHybrisEglServerBufferIntegration: Trying to use unresolved function eglHybrisGetNativeBufferInfo");
}

void LibHybrisEglServerBufferIntegration::eglHybrisSerializeNativeBuffer(EGLClientBuffer buffer, int *ints, int *fds)
{
    if (m_egl_serialize_buffer)
        m_egl_serialize_buffer(buffer, ints, fds);
    else
        qWarning("LibHybrisEglServerBufferIntegration: Trying to use unresolved function eglHybrisSerializeNativeBuffer");
}

QT_END_NAMESPACE

#endif
