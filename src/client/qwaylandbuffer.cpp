/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2017 Giulio Camuffo.
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
****************************************************************************/

#include "qwaylandbuffer_p.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandBuffer::QWaylandBuffer()
{
}

QWaylandBuffer::~QWaylandBuffer()
{
    if (mBuffer)
        wl_buffer_destroy(mBuffer);
}

void QWaylandBuffer::init(wl_buffer *buf)
{
    mBuffer = buf;
    wl_buffer_add_listener(buf, &listener, this);
}

void QWaylandBuffer::release(void *data, wl_buffer *)
{
    QWaylandBuffer *self = static_cast<QWaylandBuffer *>(data);
    self->mBusy = false;
    self->mCommitted = false;
}

const wl_buffer_listener QWaylandBuffer::listener = {
    QWaylandBuffer::release
};

}

QT_END_NAMESPACE
