/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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

#include <QtCore/QSize>
#include <QtGui/qopengl.h>

#include <QtWaylandClient/private/qwayland-server-buffer-extension.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QOpenGLTexture;

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandServerBuffer
{
public:
    enum Format {
        RGBA32,
        A8,
        Custom
    };

    QWaylandServerBuffer();
    virtual ~QWaylandServerBuffer();

    virtual QOpenGLTexture *toOpenGlTexture() = 0;

    Format format() const;
    QSize size() const;

    void setUserData(void *userData);
    void *userData() const;

protected:
    Format m_format = RGBA32;
    QSize m_size;

private:
    void *m_user_data = nullptr;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandServerBufferIntegration
{
public:
    QWaylandServerBufferIntegration();
    virtual ~QWaylandServerBufferIntegration();

    virtual void initialize(QWaylandDisplay *display) = 0;

    virtual QWaylandServerBuffer *serverBuffer(struct qt_server_buffer *buffer) = 0;
};

}

QT_END_NAMESPACE

#endif
