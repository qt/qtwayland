TEMPLATE=subdirs

SUBDIRS += qwindow-compositor
SUBDIRS += minimal-cpp

qtHaveModule(quick) {
    SUBDIRS += minimal-qml
    SUBDIRS += pure-qml
    SUBDIRS += multi-output
    SUBDIRS += custom-extension
}

SUBDIRS += server-buffer
