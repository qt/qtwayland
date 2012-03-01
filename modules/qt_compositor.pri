QT.compositor.VERSION = 5.0.0
QT.compositor.MAJOR_VERSION = 5
QT.compositor.MINOR_VERSION = 0
QT.compositor.PATCH_VERSION = 0

QT.compositor.name = QtCompositor
QT.compositor.bins = $$QT_MODULE_BIN_BASE
QT.compositor.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtCompositor
QT.compositor.private_includes = $$QT_MODULE_INCLUDE_BASE/QtCompositor/$$QT.compositor.VERSION
QT.compositor.sources = $$QT_MODULE_BASE/src/compositor
QT.compositor.libs = $$QT_MODULE_LIB_BASE
QT.compositor.plugins = $$QT_MODULE_PLUGIN_BASE
QT.compositor.imports = $$QT_MODULE_IMPORT_BASE
QT.compositor.depends = gui
contains(QT_CONFIG, opengl) {
    QT.compositor.DEFINES = QT_COMPOSITOR_WAYLAND_GL
}

QT_CONFIG += compositor
