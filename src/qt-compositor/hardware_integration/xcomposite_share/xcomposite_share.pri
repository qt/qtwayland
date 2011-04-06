INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/wayland-xcomposite-server-protocol.h \
    $$PWD/xcompositebuffer.h \
    $$PWD/xcompositehandler.h \
    $$PWD/xlibinclude.h

SOURCES += \
    $$PWD/wayland-xcomposite-protocol.c \
    $$PWD/xcompositebuffer.cpp \
    $$PWD/xcompositehandler.cpp
