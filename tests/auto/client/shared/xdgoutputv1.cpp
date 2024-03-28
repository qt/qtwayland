// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "xdgoutputv1.h"

namespace MockCompositor {

int XdgOutputV1::s_nextId = 1;

void XdgOutputV1::sendLogicalSize(const QSize &size)
{
    m_logicalGeometry.setSize(size);
    for (auto *resource : resourceMap())
        zxdg_output_v1::send_logical_size(resource->handle, size.width(), size.height());
}

void XdgOutputV1::addResource(wl_client *client, int id, int version)
{
    auto *resource = add(client, id, version)->handle;
    zxdg_output_v1::send_logical_size(resource, m_logicalGeometry.width(), m_logicalGeometry.height());
    send_logical_position(resource, m_logicalGeometry.x(), m_logicalGeometry.y());
    if (version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION)
        send_name(resource, m_name);
    if (version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION)
        send_description(resource, m_description);

    if (version < 3) // zxdg_output_v1.done has been deprecated
        zxdg_output_v1::send_done(resource);
    else {
        m_output->sendDone(client);
    }
}

} // namespace MockCompositor
