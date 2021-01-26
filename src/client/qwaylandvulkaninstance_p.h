/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of plugins of the Qt Toolkit.
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

#ifndef QWAYLANDVULKANINSTANCE_P_H
#define QWAYLANDVULKANINSTANCE_P_H

#if defined(VULKAN_H_) && !defined(VK_USE_PLATFORM_WAYLAND_KHR)
#error "vulkan.h included without Wayland WSI"
#endif

#define VK_USE_PLATFORM_WAYLAND_KHR

#include <QtVulkanSupport/private/qbasicvulkanplatforminstance_p.h>
#include <QLibrary>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;

class QWaylandVulkanInstance : public QBasicPlatformVulkanInstance
{
public:
    explicit QWaylandVulkanInstance(QVulkanInstance *instance);
    ~QWaylandVulkanInstance() override;

    void createOrAdoptInstance() override;
    bool supportsPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, QWindow *window) override;
    void presentAboutToBeQueued(QWindow *window) override;

    VkSurfaceKHR createSurface(QWaylandWindow *window);

private:
    QVulkanInstance *m_instance = nullptr;
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR m_getPhysDevPresSupport = nullptr;
    PFN_vkCreateWaylandSurfaceKHR m_createSurface = nullptr;
};

} // namespace QtWaylandClient

QT_END_NAMESPACE

#endif // QWAYLANDVULKANINSTANCE_P_H
