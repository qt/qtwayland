load(configure)
qtCompileTest(wayland)
qtCompileTest(xkbcommon)
qtCompileTest(wayland_cursor)
qtCompileTest(wayland_scanner)
qtCompileTest(wayland_egl)
qtCompileTest(egl)
qtCompileTest(brcm_egl)
qtCompileTest(glx)
qtCompileTest(xcomposite)
qtCompileTest(drm_egl_server)
qtCompileTest(libhybris_egl_server)

load(qt_parts)

!config_wayland: error("QtWayland requires Wayland 1.2.0 or higher")
!config_wayland_scanner: error("QtWayland requires wayland-scanner")
!config_wayland_cursor: error("QtWayland requires wayland-cursor")

!config_xkbcommon {
    warning("No xkbcommon 0.2.0 or higher found, disabling support for it")
}

!config_wayland_egl {
    message("no wayland-egl support detected, cross-toolkit compatibility disabled");
}
