TEMPLATE=subdirs
SUBDIRS += qwidget-compositor qwidget-compositor-mdi qwindow-compositor

contains(QT_CONFIG, quick) {
    SUBDIRS += qml-compositor
}
