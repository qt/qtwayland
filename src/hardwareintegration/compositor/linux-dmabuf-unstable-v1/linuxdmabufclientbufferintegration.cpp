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

#include "linuxdmabufclientbufferintegration.h"
#include "linuxdmabuf.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/private/qwayland-server-wayland.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unistd.h>
#include <drm_fourcc.h>

QT_BEGIN_NAMESPACE

static QWaylandBufferRef::BufferFormatEgl formatFromDrmFormat(EGLint format) {
    switch (format) {
    case DRM_FORMAT_RGB332:
    case DRM_FORMAT_BGR233:
    case DRM_FORMAT_XRGB4444:
    case DRM_FORMAT_XBGR4444:
    case DRM_FORMAT_RGBX4444:
    case DRM_FORMAT_BGRX4444:
    case DRM_FORMAT_XRGB1555:
    case DRM_FORMAT_XBGR1555:
    case DRM_FORMAT_RGBX5551:
    case DRM_FORMAT_BGRX5551:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGBX8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_RGBX1010102:
    case DRM_FORMAT_BGRX1010102:
        return QWaylandBufferRef::BufferFormatEgl_RGB;
    case DRM_FORMAT_ARGB4444:
    case DRM_FORMAT_ABGR4444:
    case DRM_FORMAT_RGBA4444:
    case DRM_FORMAT_BGRA4444:
    case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_RGBA5551:
    case DRM_FORMAT_BGRA5551:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_RGBA1010102:
    case DRM_FORMAT_BGRA1010102:
        return QWaylandBufferRef::BufferFormatEgl_RGBA;
    case DRM_FORMAT_YUYV:
        return QWaylandBufferRef::BufferFormatEgl_Y_XUXV;
    default:
        qCDebug(qLcWaylandCompositorHardwareIntegration) << "Buffer format" << Qt::hex << format << "not supported";
        return QWaylandBufferRef::BufferFormatEgl_Null;
    }
}

static QOpenGLTexture::TextureFormat openGLFormatFromBufferFormat(QWaylandBufferRef::BufferFormatEgl format) {
    switch (format) {
    case QWaylandBufferRef::BufferFormatEgl_RGB:
        return QOpenGLTexture::RGBFormat;
    case QWaylandBufferRef::BufferFormatEgl_RGBA:
        return QOpenGLTexture::RGBAFormat;
    default:
        return QOpenGLTexture::NoFormat;
    }
}

bool LinuxDmabufClientBufferIntegration::initSimpleTexture(LinuxDmabufWlBuffer *dmabufBuffer)
{
    bool success = true;

    // Resolving GL functions may need a context current, so do it only here.
    if (!gl_egl_image_target_texture_2d)
        gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

    if (dmabufBuffer->plane(0).modifiers != DRM_FORMAT_MOD_INVALID && !m_supportsDmabufModifiers) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Buffer uses dmabuf modifiers, which are not supported.";
        success = false;
    }

    for (uint32_t i = 0; i < dmabufBuffer->planesNumber(); ++i) {
        QVarLengthArray<EGLint, 17> attribs;
        switch (i) {
        case 0:
            attribs = {
                EGL_WIDTH,                          dmabufBuffer->size().width(),
                EGL_HEIGHT,                         dmabufBuffer->size().height(),
                EGL_LINUX_DRM_FOURCC_EXT,           EGLint(dmabufBuffer->drmFormat()),
                EGL_DMA_BUF_PLANE0_FD_EXT,          dmabufBuffer->plane(i).fd,
                EGL_DMA_BUF_PLANE0_OFFSET_EXT,      EGLint(dmabufBuffer->plane(i).offset),
                EGL_DMA_BUF_PLANE0_PITCH_EXT,       EGLint(dmabufBuffer->plane(i).stride),
                EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGLint(dmabufBuffer->plane(i).modifiers & 0xffffffff),
                EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, EGLint(dmabufBuffer->plane(i).modifiers >> 32),
                EGL_NONE
            };
            break;
        case 1:
            attribs = {
                EGL_WIDTH,                          dmabufBuffer->size().width(),
                EGL_HEIGHT,                         dmabufBuffer->size().height(),
                EGL_LINUX_DRM_FOURCC_EXT,           EGLint(dmabufBuffer->drmFormat()),
                EGL_DMA_BUF_PLANE1_FD_EXT,          dmabufBuffer->plane(i).fd,
                EGL_DMA_BUF_PLANE1_OFFSET_EXT,      EGLint(dmabufBuffer->plane(i).offset),
                EGL_DMA_BUF_PLANE1_PITCH_EXT,       EGLint(dmabufBuffer->plane(i).stride),
                EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT, EGLint(dmabufBuffer->plane(i).modifiers & 0xffffffff),
                EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT, EGLint(dmabufBuffer->plane(i).modifiers >> 32),
                EGL_NONE
            };
            break;
        case 2:
            attribs = {
                EGL_WIDTH,                          dmabufBuffer->size().width(),
                EGL_HEIGHT,                         dmabufBuffer->size().height(),
                EGL_LINUX_DRM_FOURCC_EXT,           EGLint(dmabufBuffer->drmFormat()),
                EGL_DMA_BUF_PLANE2_FD_EXT,          dmabufBuffer->plane(i).fd,
                EGL_DMA_BUF_PLANE2_OFFSET_EXT,      EGLint(dmabufBuffer->plane(i).offset),
                EGL_DMA_BUF_PLANE2_PITCH_EXT,       EGLint(dmabufBuffer->plane(i).stride),
                EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGLint(dmabufBuffer->plane(i).modifiers & 0xffffffff),
                EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT, EGLint(dmabufBuffer->plane(i).modifiers >> 32),
                EGL_NONE
            };
            break;
        case 3:
            attribs = {
                EGL_WIDTH,                          dmabufBuffer->size().width(),
                EGL_HEIGHT,                         dmabufBuffer->size().height(),
                EGL_LINUX_DRM_FOURCC_EXT,           EGLint(dmabufBuffer->drmFormat()),
                EGL_DMA_BUF_PLANE3_FD_EXT,          dmabufBuffer->plane(i).fd,
                EGL_DMA_BUF_PLANE3_OFFSET_EXT,      EGLint(dmabufBuffer->plane(i).offset),
                EGL_DMA_BUF_PLANE3_PITCH_EXT,       EGLint(dmabufBuffer->plane(i).stride),
                EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT, EGLint(dmabufBuffer->plane(i).modifiers & 0xffffffff),
                EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT, EGLint(dmabufBuffer->plane(i).modifiers >> 32),
                EGL_NONE
            };
        break;
        default:
            return false;
        }

        // note: EGLImageKHR does NOT take ownership of the file descriptors
        EGLImageKHR image = egl_create_image(m_eglDisplay,
                                             EGL_NO_CONTEXT,
                                             EGL_LINUX_DMA_BUF_EXT,
                                             (EGLClientBuffer) nullptr,
                                             attribs.constData());

        if (image == EGL_NO_IMAGE_KHR) {
            qCWarning(qLcWaylandCompositorHardwareIntegration) << "failed to create EGL image for plane" << i;
            success = false;
        }

        dmabufBuffer->initImage(i, image);
    }
    return success;
}

bool LinuxDmabufClientBufferIntegration::initYuvTexture(LinuxDmabufWlBuffer *dmabufBuffer)
{
    bool success = true;

    const YuvFormatConversion conversion = m_yuvFormats.value(dmabufBuffer->drmFormat());
    if (conversion.inputPlanes != dmabufBuffer->planesNumber()) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Buffer for this format must provide" << conversion.inputPlanes
                                                           << "planes but only" << dmabufBuffer->planesNumber() << "received";
        return false;
    }

    // Resolving GL functions may need a context current, so do it only here.
    if (!gl_egl_image_target_texture_2d)
        gl_egl_image_target_texture_2d = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));


    if (dmabufBuffer->plane(0).modifiers != DRM_FORMAT_MOD_INVALID && !m_supportsDmabufModifiers) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Buffer uses dmabuf modifiers, which are not supported.";
        success = false;
    }

    for (uint32_t i = 0; i < conversion.outputPlanes; ++i) {
        const YuvPlaneConversion plane = conversion.plane[i];

        QVarLengthArray<EGLint, 17> attribs = {
            EGL_WIDTH,                          dmabufBuffer->size().width() / plane.widthDivisor,
            EGL_HEIGHT,                         dmabufBuffer->size().height() / plane.heightDivisor,
            EGL_LINUX_DRM_FOURCC_EXT,           plane.format,
            EGL_DMA_BUF_PLANE0_FD_EXT,          dmabufBuffer->plane(plane.planeIndex).fd,
            EGL_DMA_BUF_PLANE0_OFFSET_EXT,      EGLint(dmabufBuffer->plane(plane.planeIndex).offset),
            EGL_DMA_BUF_PLANE0_PITCH_EXT,       EGLint(dmabufBuffer->plane(plane.planeIndex).stride),
            EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGLint(dmabufBuffer->plane(plane.planeIndex).modifiers & 0xffffffff),
            EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, EGLint(dmabufBuffer->plane(plane.planeIndex).modifiers >> 32),
            EGL_NONE
        };

        // note: EGLImageKHR does NOT take ownership of the file descriptors
        EGLImageKHR image = egl_create_image(m_eglDisplay,
                                             EGL_NO_CONTEXT,
                                             EGL_LINUX_DMA_BUF_EXT,
                                             (EGLClientBuffer) nullptr,
                                             attribs.constData());

        if (image == EGL_NO_IMAGE_KHR) {
            qCWarning(qLcWaylandCompositorHardwareIntegration) << "failed to create EGL image for plane" << i;
            success = false;
        }

        dmabufBuffer->initImage(i, image);
    }
    return success;
}

LinuxDmabufClientBufferIntegration::LinuxDmabufClientBufferIntegration()
{
    YuvPlaneConversion firstPlane;
    firstPlane.format = DRM_FORMAT_GR88;
    firstPlane.widthDivisor = 1;
    firstPlane.heightDivisor = 1;
    firstPlane.planeIndex = 0;

    YuvPlaneConversion secondPlane;
    secondPlane.format = DRM_FORMAT_ARGB8888;
    secondPlane.widthDivisor = 2;
    secondPlane.heightDivisor = 1;
    secondPlane.planeIndex = 0;

    YuvFormatConversion formatConversion;
    formatConversion.inputPlanes = 1;
    formatConversion.outputPlanes = 2;
    formatConversion.plane[0] = firstPlane;
    formatConversion.plane[1] = secondPlane;

    m_yuvFormats.insert(DRM_FORMAT_YUYV, formatConversion);
}

LinuxDmabufClientBufferIntegration::~LinuxDmabufClientBufferIntegration()
{
    m_importedBuffers.clear();
}

void LinuxDmabufClientBufferIntegration::initializeHardware(struct ::wl_display *display)
{
    m_linuxDmabuf.reset(new LinuxDmabuf(display, this));

    const bool ignoreBindDisplay = !qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").isEmpty() && qgetenv("QT_WAYLAND_IGNORE_BIND_DISPLAY").toInt() != 0;

    // initialize hardware extensions
    egl_query_dmabuf_modifiers_ext = reinterpret_cast<PFNEGLQUERYDMABUFMODIFIERSEXTPROC>(eglGetProcAddress("eglQueryDmaBufModifiersEXT"));
    egl_query_dmabuf_formats_ext = reinterpret_cast<PFNEGLQUERYDMABUFFORMATSEXTPROC>(eglGetProcAddress("eglQueryDmaBufFormatsEXT"));
    if (!egl_query_dmabuf_modifiers_ext || !egl_query_dmabuf_formats_ext) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. Could not find eglQueryDmaBufModifiersEXT and eglQueryDmaBufFormatsEXT.";
        return;
    }

    egl_bind_wayland_display = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
    egl_unbind_wayland_display = reinterpret_cast<PFNEGLUNBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
    if ((!egl_bind_wayland_display || !egl_unbind_wayland_display) && !ignoreBindDisplay) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. Could not find eglBindWaylandDisplayWL and eglUnbindWaylandDisplayWL.";
        return;
    }

    egl_create_image = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    egl_destroy_image = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!egl_create_image || !egl_destroy_image) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. Could not find eglCreateImageKHR and eglDestroyImageKHR.";
        return;
    }

    // initialize EGL display
    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. No native platform interface available.";
        return;
    }

    m_eglDisplay = nativeInterface->nativeResourceForIntegration("EglDisplay");
    if (!m_eglDisplay) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. Could not get EglDisplay for window.";
        return;
    }

    const char *extensionString = eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
    if (!extensionString || !strstr(extensionString, "EGL_EXT_image_dma_buf_import")) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. There is no EGL_EXT_image_dma_buf_import extension.";
        return;
    }
    if (strstr(extensionString, "EGL_EXT_image_dma_buf_import_modifiers"))
        m_supportsDmabufModifiers = true;

    if (egl_bind_wayland_display && egl_unbind_wayland_display) {
        m_displayBound = egl_bind_wayland_display(m_eglDisplay, display);
        if (!m_displayBound) {
            if (ignoreBindDisplay) {
                qCWarning(qLcWaylandCompositorHardwareIntegration) << "Could not bind Wayland display. Ignoring.";
            } else {
                qCWarning(qLcWaylandCompositorHardwareIntegration) << "Failed to initialize EGL display. Could not bind Wayland display.";
                return;
            }
        }
    }

    // request and sent formats/modifiers only after egl_display is bound
    QHash<uint32_t, QVector<uint64_t>> modifiers;
    for (const auto &format : supportedDrmFormats()) {
        modifiers[format] = supportedDrmModifiers(format);
    }
    m_linuxDmabuf->setSupportedModifiers(modifiers);
}

QVector<uint32_t> LinuxDmabufClientBufferIntegration::supportedDrmFormats()
{
    if (!egl_query_dmabuf_formats_ext)
        return QVector<uint32_t>();

    // request total number of formats
    EGLint count = 0;
    EGLBoolean success = egl_query_dmabuf_formats_ext(m_eglDisplay, 0, nullptr, &count);

    if (success && count > 0) {
        QVector<uint32_t> drmFormats(count);
        if (egl_query_dmabuf_formats_ext(m_eglDisplay, count, (EGLint *) drmFormats.data(), &count))
            return drmFormats;
    }

    return QVector<uint32_t>();
}

QVector<uint64_t> LinuxDmabufClientBufferIntegration::supportedDrmModifiers(uint32_t format)
{
    if (!egl_query_dmabuf_modifiers_ext)
        return QVector<uint64_t>();

    // request total number of formats
    EGLint count = 0;
    EGLBoolean success = egl_query_dmabuf_modifiers_ext(m_eglDisplay, format, 0, nullptr, nullptr, &count);

    if (success && count > 0) {
        QVector<uint64_t> modifiers(count);
        if (egl_query_dmabuf_modifiers_ext(m_eglDisplay, format, count, modifiers.data(), nullptr, &count)) {
            return modifiers;
        }
    }

    return QVector<uint64_t>();
}

void LinuxDmabufClientBufferIntegration::deleteOrphanedTextures()
{
    Q_ASSERT(QOpenGLContext::currentContext());
    qDeleteAll(m_orphanedTextures);
    m_orphanedTextures.clear();
}

void LinuxDmabufClientBufferIntegration::deleteImage(EGLImageKHR image)
{
    egl_destroy_image(m_eglDisplay, image);
}

QtWayland::ClientBuffer *LinuxDmabufClientBufferIntegration::createBufferFor(wl_resource *resource)
{
    // fallback for shared memory buffers
    if (wl_shm_buffer_get(resource))
        return nullptr;

    auto it = m_importedBuffers.find(resource);
    if (it != m_importedBuffers.end()) {
        m_importedBuffers.value(resource);
        return new LinuxDmabufClientBuffer(this, it.value()->resource()->handle, m_importedBuffers.value(resource));
    }
    qCWarning(qLcWaylandCompositorHardwareIntegration) << "could not create client buffer for dmabuf buffer";
    return nullptr;
}

bool LinuxDmabufClientBufferIntegration::importBuffer(wl_resource *resource, LinuxDmabufWlBuffer *linuxDmabufBuffer)
{
    if (m_importedBuffers.contains(resource)) {
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "buffer has already been added";
        return false;
    }
    m_importedBuffers[resource] = linuxDmabufBuffer;
    if (m_yuvFormats.contains(linuxDmabufBuffer->drmFormat()))
        return initYuvTexture(linuxDmabufBuffer);
    else
        return initSimpleTexture(linuxDmabufBuffer);
}

void LinuxDmabufClientBufferIntegration::removeBuffer(wl_resource *resource)
{
    m_importedBuffers.remove(resource);
}

LinuxDmabufClientBuffer::LinuxDmabufClientBuffer(LinuxDmabufClientBufferIntegration *integration,
                                                 wl_resource *bufferResource,
                                                 LinuxDmabufWlBuffer *dmabufBuffer)
    : ClientBuffer(bufferResource)
    , m_integration(integration)
{
    d = dmabufBuffer;
}

QOpenGLTexture *LinuxDmabufClientBuffer::toOpenGlTexture(int plane)
{
    // At this point we should have a valid OpenGL context, so it's safe to destroy textures
    m_integration->deleteOrphanedTextures();

    if (!m_buffer)
        return nullptr;

    QOpenGLTexture *texture = d->texture(plane);

    const auto target = static_cast<QOpenGLTexture::Target>(GL_TEXTURE_2D);

    if (!texture) {
        texture = new QOpenGLTexture(target);
        texture->setFormat(openGLFormatFromBufferFormat(formatFromDrmFormat(d->drmFormat())));
        texture->setSize(d->size().width(), d->size().height());
        texture->create();
        d->initTexture(plane, texture);
    }

    if (m_textureDirty) {
        texture->bind();
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_integration->gl_egl_image_target_texture_2d(target, d->image(plane));
    }
    return texture;
}

void LinuxDmabufClientBuffer::setDestroyed()
{
    m_integration->removeBuffer(m_buffer);
    ClientBuffer::setDestroyed();
}

LinuxDmabufClientBuffer::~LinuxDmabufClientBuffer()
{
    // resources are deleted by buffer_destroy_resource
    m_buffer = nullptr;
    d = nullptr;
}

QWaylandBufferRef::BufferFormatEgl LinuxDmabufClientBuffer::bufferFormatEgl() const
{
    return formatFromDrmFormat(d->drmFormat());
}

QSize LinuxDmabufClientBuffer::size() const
{
    return d->size();
}

QWaylandSurface::Origin LinuxDmabufClientBuffer::origin() const
{
    return (d->flags() & QtWaylandServer::zwp_linux_buffer_params_v1::flags_y_invert) ? QWaylandSurface::OriginBottomLeft : QWaylandSurface::OriginTopLeft;
}

QT_END_NAMESPACE
