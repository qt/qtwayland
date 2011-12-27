load(qt_module)

TARGET  = QtCompositor
QPRO_PWD   = $$PWD

# comment out the following to not use pkg-config in the pri files
CONFIG += use_pkgconfig

include (compositor.pri)
load(qt_module_config)

QT += gui-private

