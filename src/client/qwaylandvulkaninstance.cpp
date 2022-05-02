/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#include "qwaylandvulkaninstance_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylanddisplay_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandVulkanInstance::QWaylandVulkanInstance(QVulkanInstance *instance)
    : m_instance(instance)
{
    loadVulkanLibrary(QStringLiteral("vulkan"));
}

QWaylandVulkanInstance::~QWaylandVulkanInstance() = default;

void QWaylandVulkanInstance::createOrAdoptInstance()
{
    QByteArrayList extraExtensions;
    extraExtensions << QByteArrayLiteral("VK_KHR_wayland_surface");
    initInstance(m_instance, extraExtensions);

    if (!m_vkInst)
        return;

    m_getPhysDevPresSupport = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(
                m_vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
    if (!m_getPhysDevPresSupport)
        qWarning() << "Failed to find vkGetPhysicalDeviceWaylandPresentationSupportKHR";
}

bool QWaylandVulkanInstance::supportsPresent(VkPhysicalDevice physicalDevice,
                                         uint32_t queueFamilyIndex,
                                         QWindow *window)
{
    if (!m_getPhysDevPresSupport || !m_getPhysDevSurfaceSupport)
        return true;

    auto *w = static_cast<QWaylandWindow *>(window->handle());
    if (!w) {
        qWarning() << "Attempted to call supportsPresent() without a valid platform window";
        return false;
    }
    wl_display *display = w->display()->wl_display();
    bool ok = m_getPhysDevPresSupport(physicalDevice, queueFamilyIndex, display);

    VkSurfaceKHR surface = QVulkanInstance::surfaceForWindow(window);
    VkBool32 supported = false;
    m_getPhysDevSurfaceSupport(physicalDevice, queueFamilyIndex, surface, &supported);
    ok &= bool(supported);

    return ok;
}

VkSurfaceKHR QWaylandVulkanInstance::createSurface(QWaylandWindow *window)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    if (!m_createSurface) {
        m_createSurface = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(
                    m_vkGetInstanceProcAddr(m_vkInst, "vkCreateWaylandSurfaceKHR"));
    }
    if (!m_createSurface) {
        qWarning() << "Failed to find vkCreateWaylandSurfaceKHR";
        return surface;
    }

    VkWaylandSurfaceCreateInfoKHR surfaceInfo;
    memset(&surfaceInfo, 0, sizeof(surfaceInfo));
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.display = window->display()->wl_display();
    surfaceInfo.surface = window->wlSurface();
    VkResult err = m_createSurface(m_vkInst, &surfaceInfo, nullptr, &surface);
    if (err != VK_SUCCESS)
        qWarning("Failed to create Vulkan surface: %d", err);

    return surface;
}

void QWaylandVulkanInstance::presentAboutToBeQueued(QWindow *window)
{
    auto *w = static_cast<QWaylandWindow *>(window->handle());
    if (!w) {
        qWarning() << "Attempted to call presentAboutToBeQueued() without a valid platform window";
        return;
    }
    w->handleUpdate();
}

} // namespace QtWaylandClient

QT_END_NAMESPACE
