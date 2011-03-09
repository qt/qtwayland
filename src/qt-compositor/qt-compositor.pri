LIBS += -lwayland-server -lffi
unix {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += libdrm
}

SOURCES += $$PWD/qtcompositor.cpp \
        $$PWD/private/wlcompositor.cpp \
        $$PWD/private/wlsurface.cpp \
        $$PWD/private/wloutput.cpp \
        $$PWD/private/wldisplay.cpp \
        $$PWD/private/wlshmbuffer.cpp \ 

HEADERS += $$PWD/qtcompositor.h \
        $$PWD/private/wlcompositor.h \
        $$PWD/private/wlsurface.h \
        $$PWD/private/wloutput.h \
        $$PWD/private/wlshmbuffer.h \
        $$PWD/private/wldisplay.h \
        $$PWD/private/wlobject.h \

INCLUDEPATH += $$PWD/../3rdparty/wayland

wayland_egl  {
  LIBS += -lEGL -lGLESv2
  DEFINES += QT_COMPOSITOR_WAYLAND_EGL
}
