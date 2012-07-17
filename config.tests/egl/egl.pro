TARGET = egl
QT = core

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += egl
} else {
    LIBS += -lEGL
}

# Input
SOURCES += main.cpp
