LIBS += -lEGL

DEFINES += QT_COMPOSITOR_MESA_EGL

SOURCES += \
    $$PWD/mesaeglintegration.cpp

HEADERS += \
    $$PWD/mesaeglintegration.h
