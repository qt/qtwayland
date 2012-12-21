TEMPLATE=subdirs
SUBDIRS += qwindow-compositor

qtHaveModule(widgets) {
    SUBDIRS += qwidget-compositor
}

qtHaveModule(quick) {
    SUBDIRS += qml-compositor
}
