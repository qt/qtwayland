// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
