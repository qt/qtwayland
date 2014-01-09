TEMPLATE=subdirs
CONFIG+=ordered

SUBDIRS += qtwaylandscanner

#Don't build QtCompositor API unless explicitly enabled
contains(CONFIG, wayland-compositor) {
    SUBDIRS += compositor
}

SUBDIRS += client plugins
