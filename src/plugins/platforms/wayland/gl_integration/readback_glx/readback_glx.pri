HEADERS += \
    $$PWD/qwaylandreadbackglxintegration.h \
    $$PWD/qwaylandreadbackglxwindow.h \
    $$PWD/qwaylandreadbackglxcontext.h

SOURCES += \
    $$PWD/qwaylandreadbackglxintegration.cpp \
    $$PWD/qwaylandreadbackglxwindow.cpp \
    $$PWD/qwaylandreadbackglxcontext.cpp

!contains(QT_CONFIG, no-pkg-config) {
    CONFIG += link_pkgconfig
    PKGCONFIG += x11 gl
} else {
    LIBS += -lX11 -lGL
}
