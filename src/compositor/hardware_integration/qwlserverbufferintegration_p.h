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

#include <QtCore/qglobal.h>
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

class Q_WAYLAND_COMPOSITOR_EXPORT ServerBuffer
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

class Q_WAYLAND_COMPOSITOR_EXPORT ServerBufferIntegration
{
public:
    ServerBufferIntegration();
    virtual ~ServerBufferIntegration();

    virtual bool initializeHardware(QWaylandCompositor *);

    virtual bool supportsFormat(ServerBuffer::Format format) const = 0;
    virtual ServerBuffer *createServerBufferFromImage(const QImage &qimage, ServerBuffer::Format format) = 0;
    virtual ServerBuffer *createServerBufferFromData(const QByteArray &data, const QSize &size, uint glInternalFormat)
    {
        Q_UNUSED(data);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return nullptr;
    }
};

}

QT_END_NAMESPACE

#endif //QWAYLANDSERVERBUFFERINTEGRATION_H
