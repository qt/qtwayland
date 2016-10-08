TEMPLATE=subdirs

contains(QT_CONFIG, opengl) {
    SUBDIRS += qwindow-compositor
    SUBDIRS += minimal-cpp
}

contains(QT_CONFIG, opengl):qtHaveModule(quick) {
    SUBDIRS += minimal-qml
    SUBDIRS += spanning-screens
    SUBDIRS += pure-qml
    SUBDIRS += multi-output
    SUBDIRS += custom-extension
    SUBDIRS += server-buffer
    SUBDIRS += ivi-compositor
}
