TEMPLATE=subdirs

!qtHaveModule(waylandcompositor): \
    return()

qtConfig(opengl) {
    SUBDIRS += \
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
    SUBDIRS += qtshell
    qtHaveModule(waylandclient) {
        SUBDIRS += \
            custom-extension \
            custom-shell

            qtConfig(opengl) {
                SUBDIRS += \
                    server-buffer
            }
    }
}
