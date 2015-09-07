TEMPLATE=subdirs

SUBDIRS += qwindow-compositor

qtHaveModule(quick) {
    SUBDIRS += qml-compositor
}

SUBDIRS += server-buffer
SUBDIRS += pure-qml
