INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/waylandcompositor.h \
    $$PWD/waylandsurface.h \
    $$PWD/waylandinput.h

SOURCES += \
    $$PWD/waylandcompositor.cpp \
    $$PWD/waylandsurface.cpp \
    $$PWD/waylandinput.cpp

QT += core-private

contains(QT_CONFIG, quick) {
    SOURCES += $$PWD/waylandsurfaceitem.cpp
    HEADERS += $$PWD/waylandsurfaceitem.h

    DEFINES += QT_COMPOSITOR_QUICK

    QT += quick
    QT += quick-private gui-private
}

