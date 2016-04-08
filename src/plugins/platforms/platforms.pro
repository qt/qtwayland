TEMPLATE=subdirs
CONFIG+=ordered

SUBDIRS += qwayland-generic

config_wayland_egl {
    SUBDIRS += qwayland-egl
}

#The following integrations are only useful with QtWaylandCompositor
config_brcm_egl: \
    SUBDIRS += qwayland-brcm-egl

config_xcomposite {
    contains(QT_CONFIG, egl): \
        SUBDIRS += qwayland-xcomposite-egl
    !contains(QT_CONFIG, opengles2):config_glx: \
        SUBDIRS += qwayland-xcomposite-glx
}
