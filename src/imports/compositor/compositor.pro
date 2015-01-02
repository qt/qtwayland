CXX_MODULE = qml
TARGET  = qwaylandcompositorplugin
TARGETPATH = QtWayland/Compositor
IMPORT_VERSION = 1.0

SOURCES += \
    qwaylandquickcompositorplugin.cpp

DEFINES += QT_COMPOSITOR_QUICK
QT += quick-private qml-private compositor compositor-private

load(qml_plugin)
