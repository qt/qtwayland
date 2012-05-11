INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/waylandcompositor.h \
    $$PWD/waylandsurface.h \
    $$PWD/waylandinput.h \
    $$PWD/waylandsurfacenode.h \
    $$PWD/waylandsurfacetexturematerial.h

SOURCES += \
    $$PWD/waylandcompositor.cpp \
    $$PWD/waylandsurface.cpp \
    $$PWD/waylandinput.cpp \
    $$PWD/waylandsurfacenode.cpp \
    $$PWD/waylandsurfacetexturematerial.cpp

QT += core-private

contains(QT_CONFIG, quick) {
    SOURCES += $$PWD/waylandsurfaceitem.cpp
    HEADERS += $$PWD/waylandsurfaceitem.h

    DEFINES += QT_COMPOSITOR_QUICK

    QT += qml quick
    QT += quick-private gui-private
}

