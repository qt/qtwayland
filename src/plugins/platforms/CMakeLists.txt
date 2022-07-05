# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from platforms.pro.

add_subdirectory(qwayland-generic)
if(QT_FEATURE_wayland_egl)
    add_subdirectory(qwayland-egl)
endif()
if(QT_FEATURE_wayland_brcm)
    add_subdirectory(qwayland-brcm-egl)
endif()
