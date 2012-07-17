include (../xcomposite_share/xcomposite_share.pri)

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += xcomposite egl x11
} else {
    LIBS += -lXcomposite -lEGL -lX11
}

SOURCES += \
    $$PWD/qwaylandxcompositeeglcontext.cpp \
    $$PWD/qwaylandxcompositeeglintegration.cpp \
    $$PWD/qwaylandxcompositeeglwindow.cpp

HEADERS += \
    $$PWD/qwaylandxcompositeeglcontext.h \
    $$PWD/qwaylandxcompositeeglintegration.h \
    $$PWD/qwaylandxcompositeeglwindow.h
