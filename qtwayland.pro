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

load(qt_parts)

!config_wayland {
    warning("QtWayland requires Wayland 1.0.3 or higher")
    SUBDIRS =
}

!config_xkbcommon {
    warning("No xkbcommon 0.2.0 or higher found, disabling support for it")
}

!config_wayland_scanner {
    warning("QtWayland requires wayland-scanner")
    SUBDIRS =
}

!config_wayland_cursor {
    warning("QtWayland requires wayland-cursor")
    SUBDIRS =
}

!config_wayland_egl {
    message("no wayland-egl support detected, cross-toolkit compatibility disabled");
}
