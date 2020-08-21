CXX_MODULE = qml
TARGET  = qwaylandcompositorplugin
TARGETPATH = QtWayland/Compositor
QMl_IMPORT_NAME = QtWayland.Compositor
QML_IMPORT_VERSION = $$QT_VERSION

SOURCES += \
    qwaylandquickcompositorplugin.cpp

QML_FILES += \
    $$PWD/WaylandOutputWindow.qml \
    $$PWD/WaylandCursorItem.qml

OTHER_FILES += \
    qmldir \
    $$QML_FILES

QT += quick-private qml-private gui-private core-private waylandcompositor waylandcompositor-private
CONFIG += install_qml_files
load(qml_plugin)
