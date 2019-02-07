/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "vulkanserverbufferintegration.h"

#include "vulkanwrapper.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLTexture>
#include <QtGui/QOffscreenSurface>
#include <QtGui/qopengl.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

static constexpr bool extraDebug = false;

VulkanServerBuffer::VulkanServerBuffer(VulkanServerBufferIntegration *integration, const QImage &qimage, QtWayland::ServerBuffer::Format format)
    : QtWayland::ServerBuffer(qimage.size(),format)
    , m_integration(integration)
    , m_width(qimage.width())
    , m_height(qimage.height())
{
    m_format = format;
    switch (m_format) {
        case RGBA32:
            m_glInternalFormat = GL_RGBA8;
            break;
        // case A8:
        //     m_glInternalFormat = GL_R8;
        //     break;
        default:
            qWarning("VulkanServerBuffer: unsupported format");
            m_glInternalFormat = GL_RGBA8;
            break;
    }

    auto vulkanWrapper = m_integration->vulkanWrapper();
    m_vImage = vulkanWrapper->createTextureImage(qimage);
    if (m_vImage)
        m_fd = vulkanWrapper->getImageInfo(m_vImage, &m_memorySize);
}

VulkanServerBuffer::VulkanServerBuffer(VulkanServerBufferIntegration *integration, VulkanImageWrapper *vImage, uint glInternalFormat, const QSize &size)
    : QtWayland::ServerBuffer(size, QtWayland::ServerBuffer::Custom)
    , m_integration(integration)
    , m_width(size.width())
    , m_height(size.height())
    , m_vImage(vImage)
    , m_glInternalFormat(glInternalFormat)
{
    auto vulkanWrapper = m_integration->vulkanWrapper();
    m_fd = vulkanWrapper->getImageInfo(m_vImage, &m_memorySize);
}

VulkanServerBuffer::~VulkanServerBuffer()
{
    delete m_texture; //this is always nullptr for now
    auto vulkanWrapper = m_integration->vulkanWrapper();
    vulkanWrapper->freeTextureImage(m_vImage);
}

struct ::wl_resource *VulkanServerBuffer::resourceForClient(struct ::wl_client *client)
{
    auto *bufferResource = resourceMap().value(client);
    if (!bufferResource) {
        auto integrationResource = m_integration->resourceMap().value(client);
        if (!integrationResource) {
            qWarning("VulkanServerBuffer::resourceForClient: Trying to get resource for ServerBuffer. But client is not bound to the vulkan interface");
            return nullptr;
        }
        struct ::wl_resource *shm_integration_resource = integrationResource->handle;
        Resource *resource = add(client, 1);
        m_integration->send_server_buffer_created(shm_integration_resource, resource->handle, m_fd, m_width, m_height, m_memorySize, m_glInternalFormat);
        return resource->handle;
    }
    return bufferResource->handle;
}

QOpenGLTexture *VulkanServerBuffer::toOpenGlTexture()
{
    if (!m_texture) {
        //server-side texture support not implemented
        qWarning("VulkanServerBuffer::toOpenGlTexture not supported.");
    }
    return m_texture;
}

bool VulkanServerBuffer::bufferInUse()
{
    return resourceMap().count() > 0;
}

void VulkanServerBuffer::server_buffer_release(Resource *resource)
{
    qCDebug(qLcWaylandCompositorHardwareIntegration) << "server_buffer RELEASE resource" << resource->handle << wl_resource_get_id(resource->handle) << "for client" << resource->client();
    wl_resource_destroy(resource->handle);
}

VulkanServerBufferIntegration::VulkanServerBufferIntegration()
{
}

VulkanServerBufferIntegration::~VulkanServerBufferIntegration()
{
}

void VulkanServerBufferIntegration::initializeHardware(QWaylandCompositor *compositor)
{
    Q_ASSERT(QGuiApplication::platformNativeInterface());

    QtWaylandServer::zqt_vulkan_server_buffer_v1::init(compositor->display(), 1);
}

bool VulkanServerBufferIntegration::supportsFormat(QtWayland::ServerBuffer::Format format) const
{
    switch (format) {
        case QtWayland::ServerBuffer::RGBA32:
            return true;
        case QtWayland::ServerBuffer::A8:
            return false;
        default:
            return false;
    }
}

//RAII
class CurrentContext
{
public:
    CurrentContext()
    {
        if (!QOpenGLContext::currentContext()) {
            if (QOpenGLContext::globalShareContext()) {
                localContext = new QOpenGLContext;
                localContext->setShareContext(QOpenGLContext::globalShareContext());
                localContext->create();

                offscreenSurface = new QOffscreenSurface;
                offscreenSurface->setFormat(localContext->format());
                offscreenSurface->create();
                localContext->makeCurrent(offscreenSurface);
            } else {
                qCritical("VulkanServerBufferIntegration: no globalShareContext");
            }
        }
    }
    ~CurrentContext()
    {
        if (localContext) {
            localContext->doneCurrent();
            delete localContext;
            delete offscreenSurface;
        }
    }
    QOpenGLContext *context() { return localContext ? localContext : QOpenGLContext::currentContext(); }
private:
    QOpenGLContext *localContext = nullptr;
    QOffscreenSurface *offscreenSurface = nullptr;
};

QtWayland::ServerBuffer *VulkanServerBufferIntegration::createServerBufferFromImage(const QImage &qimage, QtWayland::ServerBuffer::Format format)
{
    if (!m_vulkanWrapper) {
        CurrentContext current;
        m_vulkanWrapper = new VulkanWrapper(current.context());
    }
    return new VulkanServerBuffer(this, qimage, format);
}

QtWayland::ServerBuffer *VulkanServerBufferIntegration::createServerBufferFromData(const QByteArray &data, const QSize &size, uint glInternalFormat)
{
    if (!m_vulkanWrapper) {
        CurrentContext current;
        m_vulkanWrapper = new VulkanWrapper(current.context());
    }

    auto *vImage = m_vulkanWrapper->createTextureImageFromData(reinterpret_cast<const uchar*>(data.constData()), data.size(), size, glInternalFormat);

    if (vImage)
        return new VulkanServerBuffer(this, vImage, glInternalFormat, size);

    qCWarning(qLcWaylandCompositorHardwareIntegration) << "could not load compressed texture";
    return nullptr;
}

QT_END_NAMESPACE
