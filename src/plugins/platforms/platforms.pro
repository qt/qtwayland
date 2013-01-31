TEMPLATE=subdirs
CONFIG+=ordered
SUBDIRS += wayland_common

equals(QT_WAYLAND_GL_CONFIG, nogl) {
    SUBDIRS += qwayland-nogl
} else {
    config_wayland_egl {
        SUBDIRS += qwayland-egl
    }

    #The following integrations are only useful with QtCompositor
    contains(CONFIG, wayland-compositor) {
        config_brcm_egl {
            SUBDIRS += qwayland-brcm-egl
        }
        config_xcomposite {
            config_egl {
                SUBDIRS += qwayland-xcomposite-egl
            } else:config_glx {
                SUBDIRS += qwayland-xcomposite-glx
            }
        }
    }
}
