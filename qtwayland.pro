load(configure)
qtCompileTest(wayland)
qtCompileTest(xkbcommon)
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
    warning("QtWayland requires xkbcommon 0.2.0 or higher")
    SUBDIRS =
}

!config_wayland_scanner {
    warning("QtWayland requires wayland-scanner")
    SUBDIRS =
}

!config_wayland_egl {
    message("no wayland-egl support detected, cross-toolkit compatibility disabled");
}
