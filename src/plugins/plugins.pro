TEMPLATE=subdirs
SUBDIRS += platforms

#The compositor plugins are only useful with QtCompositor
contains(CONFIG, wayland-compositor) {
        SUBDIRS += waylandcompositors
}
