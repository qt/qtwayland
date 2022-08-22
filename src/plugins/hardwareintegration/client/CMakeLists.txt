# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from client.pro.

if(QT_FEATURE_wayland_egl)
    add_subdirectory(wayland-egl)
endif()
if(QT_FEATURE_wayland_brcm)
    add_subdirectory(brcm-egl)
endif()
if(QT_FEATURE_wayland_drm_egl_server_buffer)
    add_subdirectory(drm-egl-server)
endif()
if(QT_FEATURE_wayland_libhybris_egl_server_buffer)
    add_subdirectory(libhybris-egl-server)
endif()
if(QT_FEATURE_wayland_shm_emulation_server_buffer)
    add_subdirectory(shm-emulation-server)
endif()
if(QT_FEATURE_wayland_dmabuf_server_buffer)
    add_subdirectory(dmabuf-server)
endif()
if(QT_FEATURE_wayland_vulkan_server_buffer)
    add_subdirectory(vulkan-server)
endif()
