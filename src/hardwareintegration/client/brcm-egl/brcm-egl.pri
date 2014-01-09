INCLUDEPATH += $$PWD
LIBS += -lEGL

SOURCES += $$PWD/qwaylandbrcmeglintegration.cpp \
           $$PWD/qwaylandbrcmglcontext.cpp \
           $$PWD/qwaylandbrcmeglwindow.cpp

HEADERS += $$PWD/qwaylandbrcmeglintegration.h \
           $$PWD/qwaylandbrcmglcontext.h \
           $$PWD/qwaylandbrcmeglwindow.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += ../../../extensions/brcm.xml
