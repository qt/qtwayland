INCLUDEPATH += compositor_api

HEADERS += \
    compositor_api/qwaylandcompositor.h \
    compositor_api/qwaylandsurface.h \
    compositor_api/qwaylandinput.h

SOURCES += \
    compositor_api/qwaylandcompositor.cpp \
    compositor_api/qwaylandsurface.cpp \
    compositor_api/qwaylandinput.cpp

QT += core-private

qtHaveModule(quick) {
    SOURCES += \
        compositor_api/qwaylandsurfaceitem.cpp \
        compositor_api/qwaylandsurfacenode.cpp

    HEADERS += \
        compositor_api/qwaylandsurfaceitem.h \
        compositor_api/qwaylandsurfacenode_p.h

    DEFINES += QT_COMPOSITOR_QUICK

    QT += qml quick
    QT += quick-private gui-private
}
