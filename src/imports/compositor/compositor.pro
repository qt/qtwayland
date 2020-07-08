CXX_MODULE = qml
TARGET  = qwaylandcompositorplugin
TARGETPATH = QtWayland/Compositor
QMl_IMPORT_NAME = QtWayland.Compositor
QML_IMPORT_VERSION = $$QT_VERSION

SOURCES += \
    qwaylandquickcompositorplugin.cpp

QT += quick-private qml-private gui-private core-private waylandcompositor waylandcompositor-private
load(qml_plugin)
