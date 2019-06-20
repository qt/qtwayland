TEMPLATE = subdirs

qtHaveModule(quick): {
    SUBDIRS += \
        compositor

    qtConfig(opengl): {
        SUBDIRS += \
            texture-sharing \
            texture-sharing-extension
    }
}
