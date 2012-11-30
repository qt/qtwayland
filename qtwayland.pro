load(configure)
qtCompileTest(wayland)
qtCompileTest(xkbcommon)
qtCompileTest(brcm_egl)
qtCompileTest(egl)
qtCompileTest(glx)
qtCompileTest(wayland_egl)
qtCompileTest(xcomposite)

load(qt_parts)

!config_wayland {
    error(QtWayland requires Wayland 1.0.0 or higher)
}
