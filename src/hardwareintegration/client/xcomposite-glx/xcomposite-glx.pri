INCLUDEPATH += $$PWD
include ($$PWD/../xcomposite_share/xcomposite_share.pri)

QMAKE_USE += wayland-client libdl

SOURCES += \
    $$PWD/qwaylandxcompositeglxcontext.cpp \
    $$PWD/qwaylandxcompositeglxintegration.cpp \
    $$PWD/qwaylandxcompositeglxwindow.cpp

HEADERS += \
    $$PWD/qwaylandxcompositeglxcontext.h \
    $$PWD/qwaylandxcompositeglxintegration.h \
    $$PWD/qwaylandxcompositeglxwindow.h
