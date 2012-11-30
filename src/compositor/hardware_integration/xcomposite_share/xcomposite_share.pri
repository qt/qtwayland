INCLUDEPATH += $$PWD

WAYLANDSOURCES += $$PWD/../../../extensions/xcomposite.xml

HEADERS += \
    $$PWD/xcompositebuffer.h \
    $$PWD/xcompositehandler.h \
    $$PWD/xlibinclude.h

SOURCES += \
    $$PWD/xcompositebuffer.cpp \
    $$PWD/xcompositehandler.cpp

QT += gui-private
