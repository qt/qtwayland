// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDBRCMEGLPLATFORMINTEGRATION_H
#define QWAYLANDBRCMEGLPLATFORMINTEGRATION_H

#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include "qwaylandbrcmeglintegration.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBrcmEglPlatformIntegration : public QWaylandIntegration
{
public:
    QWaylandBrcmEglPlatformIntegration()
        : QWaylandIntegration()
        , m_gl_integration(new QWaylandBrcmEglIntegration())
    {
        m_gl_integration->initialize(display());
    }

    QWaylandClientBufferIntegration *clientBufferIntegration() const override
    {
        return m_gl_integration;
    }
private:
    QWaylandClientBufferIntegration *m_gl_integration;
};

}

QT_END_NAMESPACE

#endif
