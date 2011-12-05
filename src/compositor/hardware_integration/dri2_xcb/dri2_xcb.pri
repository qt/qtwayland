INCLUDEPATH += $$PWD/../private
LIBS += -lxcb -lxcb-dri2 -lEGL

DEFINES += QT_COMPOSITOR_DRI2_XCB

SOURCES += \
    $$PWD/dri2xcbhwintegration.cpp \
    $$PWD/dri2xcbbuffer.cpp \
    $$PWD/../../../3rdparty/wayland/wayland-drm-protocol.c \

HEADERS += \
    $$PWD/dri2xcbhwintegration.h \
    $$PWD/dri2xcbbuffer.h \
    $$PWD/../../../3rdparty/wayland/wayland-drm-server-protocol.h \
