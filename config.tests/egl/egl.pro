TARGET = egl
QT = core

!contains(QT_CONFIG, opengles2):error("wayland_egl support currently requires Qt configured with OpenGL ES2")

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += egl
} else {
    LIBS += -lEGL
}

# Input
SOURCES += main.cpp
