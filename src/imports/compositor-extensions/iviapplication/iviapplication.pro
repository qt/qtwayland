CXX_MODULE = qml
TARGET  = qwaylandcompositoriviapplicationplugin
TARGETPATH = QtWayland/Compositor/IviApplication
QML_IMPORT_VERSION = $$QT_VERSION

SOURCES += \
    qwaylandcompositoriviapplicationplugin.cpp

QT += waylandcompositor

load(qml_plugin)
