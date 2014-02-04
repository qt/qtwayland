INCLUDEPATH += compositor_api

HEADERS += \
    compositor_api/qwaylandcompositor.h \
    compositor_api/qwaylandsurface.h \
    compositor_api/qwaylandinput.h \
    compositor_api/qwaylandinputpanel.h \
    compositor_api/qwaylanddrag.h

SOURCES += \
    compositor_api/qwaylandcompositor.cpp \
    compositor_api/qwaylandsurface.cpp \
    compositor_api/qwaylandinput.cpp \
    compositor_api/qwaylandinputpanel.cpp \
    compositor_api/qwaylanddrag.cpp

QT += core-private

qtHaveModule(quick) {
    SOURCES += \
        compositor_api/qwaylandsurfaceitem.cpp

    HEADERS += \
        compositor_api/qwaylandsurfaceitem.h

    DEFINES += QT_COMPOSITOR_QUICK

    QT += qml quick
    QT += quick-private gui-private
}
