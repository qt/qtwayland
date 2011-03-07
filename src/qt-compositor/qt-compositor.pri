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
        $$PWD/wlshmbuffer.cpp 

HEADERS += $$PWD/qtcompositor.h \
        $$PWD/wlcompositor.h \
        $$PWD/wlsurface.h \
        $$PWD/wloutput.h \
        $$PWD/wlshmbuffer.h \
        $$PWD/wldisplay.h \
        $$PWD/wlobject.h

INCLUDEPATH += $$PWD/../3rdparty/wayland

wayland_egl  {
  LIBS += -lEGL -lGLESv2
  DEFINES += QT_COMPOSITOR_WAYLAND_EGL
}
