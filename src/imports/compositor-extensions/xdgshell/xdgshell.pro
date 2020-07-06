CXX_MODULE = qml
TARGET  = qwaylandcompositorxdgshellplugin
TARGETPATH = QtWayland/Compositor/XdgShell
QML_IMPORT_VERSION = $$QT_VERSION

SOURCES += \
    qwaylandcompositorxdgshellplugin.cpp

QT += waylandcompositor

load(qml_plugin)
