TEMPLATE=subdirs
SUBDIRS += qwindow-compositor

!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += qwidget-compositor
}

contains(QT_CONFIG, quick) {
    SUBDIRS += qml-compositor
}
