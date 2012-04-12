TARGET = xkbcommon
QT = core

CONFIG += link_pkgconfig

!contains(QT_CONFIG, no-pkg-config) {
    PKGCONFIG += xkbcommon
} else {
    LIBS += -lxkbcommon
}

# Input
SOURCES += main.cpp
