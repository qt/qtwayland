LIBS += -lEGL

DEFINES += QT_COMPOSITOR_MESA_EGL

SOURCES += \
    $$PWD/waylandeglintegration.cpp

HEADERS += \
    $$PWD/waylandeglintegration.h
