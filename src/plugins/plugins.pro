TEMPLATE=subdirs

qtHaveModule(waylandclient) {
    SUBDIRS += \
        platforms \
        decorations \
        shellintegration
}

SUBDIRS += \
    hardwareintegration
