LIBS += -lEGL

SOURCES += \
    $$PWD/brcmeglintegration.cpp \
    $$PWD/brcmbuffer.cpp

HEADERS += \
    $$PWD/brcmeglintegration.h \
    $$PWD/brcmbuffer.h

WAYLANDSOURCES += $$PWD/../../../../extensions/brcm.xml
