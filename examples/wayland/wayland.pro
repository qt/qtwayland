TEMPLATE=subdirs

SUBDIRS += qwindow-compositor

qtHaveModule(quick) {
    SUBDIRS += pure-qml
}

SUBDIRS += server-buffer
