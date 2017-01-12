requires(qtHaveModule(waylandcompositor))
TEMPLATE=subdirs

qtHaveModule(opengl) {
    SUBDIRS += \
        qwindow-compositor \
        minimal-cpp
}

qtHaveModule(quick) {
    SUBDIRS += minimal-qml
    SUBDIRS += spanning-screens
    SUBDIRS += pure-qml
    SUBDIRS += multi-output
    SUBDIRS += multi-screen
    SUBDIRS += overview-compositor
    SUBDIRS += ivi-compositor
    SUBDIRS += server-side-decoration
    qtHaveModule(waylandclient) {
        SUBDIRS += \
            custom-extension

            qtHaveModule(opengl) {
                SUBDIRS += \
                    server-buffer \
                    texture-sharing
            }
    }
    SUBDIRS += hwlayer-compositor
}
