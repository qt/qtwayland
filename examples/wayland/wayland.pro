TEMPLATE=subdirs

SUBDIRS += qwindow-compositor

qtHaveModule(quick) {
    SUBDIRS += pure-qml
    SUBDIRS += multi-output
    SUBDIRS += custom-extension
}

SUBDIRS += server-buffer
