TEMPLATE=subdirs

SUBDIRS += qwindow-compositor

qtHaveModule(quick) {
    SUBDIRS += pure-qml
    SUBDIRS += multi-output
}

SUBDIRS += server-buffer
