load(qt_module)

TARGET  = QtCompositor
QPRO_PWD   = $$PWD

include (compositor.pri)
load(qt_module_config)

QT += gui-private

