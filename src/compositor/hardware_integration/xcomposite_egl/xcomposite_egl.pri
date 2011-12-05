include (../xcomposite_share/xcomposite_share.pri)

LIBS += -lXcomposite -lX11 -lEGL

HEADERS += \
    $$PWD/xcompositeeglintegration.h

SOURCES += \
    $$PWD/xcompositeeglintegration.cpp
