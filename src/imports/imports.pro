TEMPLATE = subdirs

qtHaveModule(quick):qtHaveModule(waylandcompositor) {
    SUBDIRS += \
        compositor \
        compositor-extensions

    qtConfig(opengl):qtHaveModule(waylandclient) {
        SUBDIRS += \
            texture-sharing \
            texture-sharing-extension
    }
}
