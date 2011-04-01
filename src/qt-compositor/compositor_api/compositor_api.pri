INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/waylandcompositor.h \
    $$PWD/waylandsurface.h

SOURCES += \
    $$PWD/waylandcompositor.cpp \
    $$PWD/waylandsurface.cpp

contains(QT, declarative) {
    echo("Got here!");
    SOURCES += $$PWD/waylandsurfaceitem.cpp
    HEADERS += $$PWD/waylandsurfaceitem.h
}
