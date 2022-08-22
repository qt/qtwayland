# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from shellintegration.pro.

if(QT_FEATURE_wayland_client_fullscreen_shell_v1)
    add_subdirectory(fullscreen-shell-v1)
endif()
if(QT_FEATURE_wayland_client_ivi_shell)
    add_subdirectory(ivi-shell)
endif()
if(QT_FEATURE_wayland_client_wl_shell)
    add_subdirectory(wl-shell)
endif()
if(QT_FEATURE_wayland_client_xdg_shell)
    add_subdirectory(xdg-shell)
endif()

if(QT_FEATURE_wayland_client_qt_shell)
    add_subdirectory(qt-shell)
endif()
