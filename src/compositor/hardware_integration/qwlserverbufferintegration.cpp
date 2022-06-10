// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwlserverbufferintegration_p.h"

QT_BEGIN_NAMESPACE

namespace QtWayland {

ServerBuffer::ServerBuffer(const QSize &size, ServerBuffer::Format format)
    : m_size(size)
    , m_format(format)
{ }
ServerBuffer::~ServerBuffer()
{ }


bool ServerBuffer::isYInverted() const
{
    return false;
}

QSize ServerBuffer::size() const
{ return m_size; }

ServerBuffer::Format ServerBuffer::format() const
{ return m_format; }

ServerBufferIntegration::ServerBufferIntegration()
{ }

ServerBufferIntegration::~ServerBufferIntegration()
{ }

bool ServerBufferIntegration::initializeHardware(QWaylandCompositor *compositor)
{
    Q_UNUSED(compositor);
    return true;
}

}

QT_END_NAMESPACE
