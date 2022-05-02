/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
