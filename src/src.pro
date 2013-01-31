TEMPLATE=subdirs
CONFIG+=ordered

#Don't build QtCompositor API unless explicitly enabled
contains(CONFIG, wayland-compositor) {
    SUBDIRS += compositor
}

SUBDIRS += plugins
