TEMPLATE = subdirs

!isEqual(QT_WAYLAND_GL_CONFIG,nogl) {
    config_wayland_egl {
        SUBDIRS += wayland-egl
    }
    config_brcm_egl {
        SUBDIRS += brcm-egl
    }
    config_xcomposite {
        config_egl {
            SUBDIRS += xcomposite-egl
        } else:config_glx {
            SUBDIRS += xcomposite-glx
        }
    }
}
