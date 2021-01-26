/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "qwaylandvulkanwindow_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandVulkanWindow::QWaylandVulkanWindow(QWindow *window, QWaylandDisplay *display)
    : QWaylandWindow(window, display)
{
}

QWaylandVulkanWindow::~QWaylandVulkanWindow()
{
    if (m_surface) {
        QVulkanInstance *inst = window()->vulkanInstance();
        if (inst)
            static_cast<QWaylandVulkanInstance *>(inst->handle())->destroySurface(m_surface);
    }
}

QWaylandWindow::WindowType QWaylandVulkanWindow::windowType() const
{
    return QWaylandWindow::Vulkan;
}

VkSurfaceKHR *QWaylandVulkanWindow::surface()
{
    if (m_surface)
        return &m_surface;

    QVulkanInstance *vulkanInstance = window()->vulkanInstance();
    if (!vulkanInstance) {
        qWarning() << "Attempted to create Vulkan surface without an instance; was QWindow::setVulkanInstance() called?";
        return nullptr;
    }

    auto *waylandVulkanInstance = static_cast<QWaylandVulkanInstance *>(vulkanInstance->handle());
    m_surface = waylandVulkanInstance->createSurface(this);

    return &m_surface;
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
