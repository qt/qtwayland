TEMPLATE=subdirs

config_wayland_egl: \
    SUBDIRS += wayland-egl

config_brcm_egl: \
    SUBDIRS += brcm-egl

config_xcomposite {
    config_egl: \
        SUBDIRS += xcomposite-egl

    !contains(QT_CONFIG, opengles2):config_glx: \
        SUBDIRS += xcomposite-glx
}

# we should try and resolve all symbols
# and they should be type deffed if not present
SUBDIRS += drm-egl-server
