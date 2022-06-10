// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MOCKCOMPOSITOR_XDGOUTPUTV1_H
#define MOCKCOMPOSITOR_XDGOUTPUTV1_H

#include "coreprotocol.h"

#include <qwayland-server-xdg-output-unstable-v1.h>

namespace MockCompositor {

class XdgOutputV1 : public QObject, public QtWaylandServer::zxdg_output_v1
{
public:
    explicit XdgOutputV1(Output *output)
        : m_output(output)
        , m_logicalGeometry(m_output->m_data.position, QSize(m_output->m_data.mode.resolution / m_output->m_data.scale))
        , m_name(QString("WL-%1").arg(s_nextId++))
    {}

    void send_logical_size(int32_t width, int32_t height) = delete;
    void sendLogicalSize(const QSize &size);

    void send_done() = delete; // zxdg_output_v1.done has been deprecated (in protocol version 3)

    void addResource(wl_client *client, int id, int version);
    Output *m_output = nullptr;
    QRect m_logicalGeometry;
    QString m_name;
    QString m_description = "This is an Xdg Output description";
    static int s_nextId;
};

class XdgOutputManagerV1 : public Global, public QtWaylandServer::zxdg_output_manager_v1
{
    Q_OBJECT
public:
    explicit XdgOutputManagerV1(CoreCompositor *compositor, int version = 3)
        : QtWaylandServer::zxdg_output_manager_v1(compositor->m_display, version)
        , m_version(version)
    {}
    int m_version = 1; // TODO: remove on libwayland upgrade
    QMap<Output *, XdgOutputV1 *> m_xdgOutputs;
    XdgOutputV1 *getXdgOutput(Output *output)
    {
        if (auto *xdgOutput = m_xdgOutputs.value(output))
            return xdgOutput;
        return m_xdgOutputs[output] = new XdgOutputV1(output); // TODO: free memory
    }

protected:
    void zxdg_output_manager_v1_get_xdg_output(Resource *resource, uint32_t id, wl_resource *outputResource) override
    {
        auto *output = fromResource<Output>(outputResource);
        auto *xdgOutput = getXdgOutput(output);
        xdgOutput->addResource(resource->client(), id, resource->version());
    }
};

} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_XDGOUTPUTV1_H
