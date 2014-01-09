TEMPLATE=subdirs

#Only build compositor examples if we are building
#the QtCompositor API
contains(CONFIG, wayland-compositor) {
    SUBDIRS += qwindow-compositor

    qtHaveModule(widgets) {
        SUBDIRS += qwidget-compositor
    }

    qtHaveModule(quick) {
        SUBDIRS += qml-compositor
    }

    SUBDIRS += server-buffer
}
