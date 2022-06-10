// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSERVERBUFFERINTEGRATION_H
#define QWAYLANDSERVERBUFFERINTEGRATION_H

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

#include <QtCore/private/qglobal_p.h>
#include <QtCore/QSize>
#include <QtGui/qopengl.h>

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

struct wl_client;
struct wl_resource;

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QOpenGLContext;
class QOpenGLTexture;
class QImage;

namespace QtWayland {
class Display;

class Q_WAYLANDCOMPOSITOR_EXPORT ServerBuffer
{
public:
    enum Format {
        RGBA32,
        A8,
        Custom
    };

    ServerBuffer(const QSize &size, ServerBuffer::Format format);
    virtual ~ServerBuffer();

    virtual struct ::wl_resource *resourceForClient(struct ::wl_client *) = 0;
    virtual bool bufferInUse() { return true; }

    virtual QOpenGLTexture *toOpenGlTexture() = 0;
    virtual void releaseOpenGlTexture() {}

    virtual bool isYInverted() const;

    QSize size() const;
    Format format() const;
protected:
    QSize m_size;
    Format m_format;
};

class Q_WAYLANDCOMPOSITOR_EXPORT ServerBufferIntegration
{
public:
    ServerBufferIntegration();
    virtual ~ServerBufferIntegration();

    virtual bool initializeHardware(QWaylandCompositor *);

    virtual bool supportsFormat(ServerBuffer::Format format) const = 0;
    virtual ServerBuffer *createServerBufferFromImage(const QImage &qimage, ServerBuffer::Format format) = 0;
    virtual ServerBuffer *createServerBufferFromData(QByteArrayView view, const QSize &size,
                                                     uint glInternalFormat)
    {
        Q_UNUSED(view);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return nullptr;
    }
};

}

QT_END_NAMESPACE

#endif //QWAYLANDSERVERBUFFERINTEGRATION_H
