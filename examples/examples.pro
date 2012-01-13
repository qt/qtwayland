TEMPLATE=subdirs
SUBDIRS += qwidget-compositor qwindow-compositor

contains(QT_CONFIG, quick) {
    SUBDIRS += qml-compositor
}
