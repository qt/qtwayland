CXX_MODULE = qml
TARGET  = qwaylandcompositorwlshellplugin
TARGETPATH = QtWayland/Compositor/WlShell
QML_IMPORT_VERSION = $$QT_VERSION

SOURCES += \
    qwaylandcompositorwlshellplugin.cpp

QT += waylandcompositor

load(qml_plugin)
