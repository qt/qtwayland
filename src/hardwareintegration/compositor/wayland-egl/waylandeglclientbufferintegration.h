/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WAYLANDEGLINTEGRATION_H
#define WAYLANDEGLINTEGRATION_H

#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class WaylandEglClientBufferIntegrationPrivate;

class WaylandEglClientBufferIntegration : public QtWayland::ClientBufferIntegration
{
    Q_DECLARE_PRIVATE(WaylandEglClientBufferIntegration)
public:
    WaylandEglClientBufferIntegration();

    void initializeHardware(struct ::wl_display *display) Q_DECL_OVERRIDE;

    void initializeBuffer(struct ::wl_resource *buffer) Q_DECL_OVERRIDE;
    QWaylandBufferRef::BufferFormatEgl bufferFormat(struct ::wl_resource *buffer) Q_DECL_OVERRIDE;
    uint textureForBuffer(struct ::wl_resource *buffer, int plane) Q_DECL_OVERRIDE;
    void bindTextureToBuffer(struct ::wl_resource *buffer) Q_DECL_OVERRIDE;
    void updateTextureForBuffer(struct ::wl_resource *buffer) Q_DECL_OVERRIDE;

    QWaylandSurface::Origin origin(struct ::wl_resource *) const Q_DECL_OVERRIDE;

    void *lockNativeBuffer(struct ::wl_resource *buffer) const Q_DECL_OVERRIDE;
    void unlockNativeBuffer(void *native_buffer) const Q_DECL_OVERRIDE;

    QSize bufferSize(struct ::wl_resource *buffer) const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(WaylandEglClientBufferIntegration)
    QScopedPointer<WaylandEglClientBufferIntegrationPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // WAYLANDEGLINTEGRATION_H
