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
        $$PWD/wlshmbuffer.cpp 

HEADERS += $$PWD/qtcompositor.h \
        $$PWD/wlcompositor.h \
        $$PWD/wlsurface.h \
        $$PWD/wloutput.h \
        $$PWD/wlbuffer.h \
        $$PWD/wlshmbuffer.h \
        $$PWD/wldisplay.h \
        $$PWD/wlobject.h

wayland_drm  {
  SOURCES += $$PWD/wldrmbuffer.cpp
  HEADERS +=  $$PWD/wldrmbuffer.h

  DEFINES += QT_WAYLAND_DRM
}
