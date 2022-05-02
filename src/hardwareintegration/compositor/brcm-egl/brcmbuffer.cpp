/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
******************************************************************************/

#include "brcmbuffer.h"

#include <EGL/eglext.h>

#include <EGL/eglext_brcm.h>

QT_BEGIN_NAMESPACE

BrcmBuffer::BrcmBuffer(struct ::wl_client *client, uint32_t id, const QSize &size, EGLint *data, size_t count)
    : QtWaylandServer::wl_buffer(client, id, 1)
    , m_handle(count)
    , m_size(size)
{
    for (size_t i = 0; i < count; ++i)
        m_handle[i] = data[i];
}

BrcmBuffer::~BrcmBuffer()
{
    static PFNEGLDESTROYGLOBALIMAGEBRCMPROC eglDestroyGlobalImage =
        (PFNEGLDESTROYGLOBALIMAGEBRCMPROC) eglGetProcAddress("eglDestroyGlobalImageBRCM");
    eglDestroyGlobalImage(handle());
}

void BrcmBuffer::buffer_destroy_resource(Resource *)
{
    delete this;
}

void BrcmBuffer::buffer_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

QT_END_NAMESPACE
