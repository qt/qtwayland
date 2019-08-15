TEMPLATE = subdirs

qtHaveModule(quick):qtHaveModule(waylandcompositor) {
    SUBDIRS += \
        compositor

    qtConfig(opengl):qtHaveModule(waylandclient) {
        SUBDIRS += \
            texture-sharing \
            texture-sharing-extension
    }
}
