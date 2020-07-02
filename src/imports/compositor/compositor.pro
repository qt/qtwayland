CXX_MODULE = qml
TARGET  = qwaylandcompositorplugin
TARGETPATH = QtWayland/Compositor
QML_IMPORT_VERSION = $$QT_VERSION

HEADERS += \
    qwaylandmousetracker_p.h

SOURCES += \
    qwaylandquickcompositorplugin.cpp \
    qwaylandmousetracker.cpp

RESOURCES += compositor.qrc

# In case of a debug build, deploy the QML files too
CONFIG(debug, debug|release): \
    QML_FILES += \
        WaylandOutputWindow.qml \
        WaylandCursorItem.qml

QT += quick-private qml-private gui-private core-private waylandcompositor waylandcompositor-private

QMAKE_QMLPLUGINDUMP_FLAGS = -defaultplatform
load(qml_plugin)
