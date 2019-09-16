TEMPLATE=subdirs

!qtHaveModule(waylandcompositor): \
    return()

qtConfig(opengl) {
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

            qtConfig(opengl) {
                SUBDIRS += \
                    server-buffer \
                    texture-sharing
            }
    }
    SUBDIRS += hwlayer-compositor
}
