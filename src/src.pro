TEMPLATE=subdirs

SUBDIRS = plugins

#Don't build QtCompositor API unless explicitly enabled
contains(CONFIG, wayland-compositor) {
    SUBDIRS += compositor
}
