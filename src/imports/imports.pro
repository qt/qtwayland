TEMPLATE = subdirs

qtHaveModule(quick): {
    SUBDIRS += \
        compositor

    qtHaveModule(opengl): {
        SUBDIRS += \
            texture-sharing \
            texture-sharing-extension
    }
}
