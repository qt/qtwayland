LIBS += -lwayland-server -lffi
unix {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += libdrm
}

SOURCES += $$PWD/qtcompositor.cpp \
        $$PWD/wlcompositor.cpp \
        $$PWD/wlsurface.cpp \
        $$PWD/wloutput.cpp \
        $$PWD/wldisplay.cpp \
        $$PWD/wlbuffer.cpp \
        $$PWD/wlshmbuffer.cpp \
        $$PWD/wldrmbuffer.cpp

HEADERS += $$PWD/qtcompositor.h \
        $$PWD/wlcompositor.h \
        $$PWD/wlsurface.h \
        $$PWD/wloutput.h \
        $$PWD/wlbuffer.h \
        $$PWD/wlshmbuffer.h \
        $$PWD/wldrmbuffer.h \
        $$PWD/wldisplay.h \
        $$PWD/wlobject.h

