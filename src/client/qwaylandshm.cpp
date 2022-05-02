/****************************************************************************
**
** Copyright (C) 2016 LG Electronics Inc, author: <mikko.levonmaa@lge.com>
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
#include <QtWaylandClient/private/qwaylandshm_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>

#include "qwaylandsharedmemoryformathelper_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandShm::QWaylandShm(QWaylandDisplay *display, int version, uint32_t id)
    : QtWayland::wl_shm(display->wl_registry(), id, qMin(version, 1))
{
}

QWaylandShm::~QWaylandShm()
{

}

void QWaylandShm::shm_format(uint32_t format)
{
    m_formats << format;
}

bool QWaylandShm::formatSupported(wl_shm_format format) const
{
    return m_formats.contains(format);
}

bool QWaylandShm::formatSupported(QImage::Format format) const
{
    wl_shm_format fmt = formatFrom(format);
    return formatSupported(fmt);
}

wl_shm_format QWaylandShm::formatFrom(QImage::Format format)
{
    return QWaylandSharedMemoryFormatHelper::fromQImageFormat(format);
}

QImage::Format QWaylandShm::formatFrom(wl_shm_format format)
{
    return QWaylandSharedMemoryFormatHelper::fromWaylandShmFormat(format);
}

}

QT_END_NAMESPACE
