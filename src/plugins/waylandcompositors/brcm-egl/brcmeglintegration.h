/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BRCMEGLINTEGRATION_H
#define BRCMEGLINTEGRATION_H

#include <QtCompositor/qwaylandgraphicshardwareintegration.h>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class BrcmEglIntegrationPrivate;

class BrcmEglIntegration : public QWaylandGraphicsHardwareIntegration
{
    Q_DECLARE_PRIVATE(BrcmEglIntegration)
public:
    BrcmEglIntegration();

    void initializeHardware(QtWayland::Display *waylandDisplay);

    GLuint createTextureFromBuffer(wl_buffer *buffer, QOpenGLContext *context);
    bool isYInverted(struct wl_buffer *) const;

    static void create_buffer(struct wl_client *client,
                          struct wl_resource *brcm,
                          uint32_t id,
                          int32_t width,
                          int32_t height,
                          wl_array *data);

    static void brcm_bind_func(struct wl_client *client, void *data, uint32_t version, uint32_t id);

private:
    Q_DISABLE_COPY(BrcmEglIntegration)
    QScopedPointer<BrcmEglIntegrationPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // BRCMEGLINTEGRATION_H

