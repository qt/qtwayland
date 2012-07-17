!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += egl x11 xext
} else {
    LIBS += -lX11 -lXext -lEGL
}

load(qpa/egl/convenience)
HEADERS += \
    $$PWD/qwaylandreadbackeglintegration.h \
    $$PWD/qwaylandreadbackeglcontext.h \
    $$PWD/qwaylandreadbackeglwindow.h \

SOURCES += \
    $$PWD/qwaylandreadbackeglintegration.cpp \
    $$PWD/qwaylandreadbackeglwindow.cpp \
    $$PWD/qwaylandreadbackeglcontext.cpp \
