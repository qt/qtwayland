TEMPLATE=subdirs
CONFIG+=ordered

equals(QT_WAYLAND_GL_CONFIG, nogl) {
    SUBDIRS += qwayland-nogl
} else {
    config_wayland_egl {
        SUBDIRS += qwayland-egl
    }

}
