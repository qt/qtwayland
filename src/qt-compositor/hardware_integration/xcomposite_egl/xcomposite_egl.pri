include (../xcomposite_share/xcomposite_share.pri)

LIBS += -lXcomposite -lEGL

HEADERS += \
    $$PWD/xcompositeeglintegration.h

SOURCES += \
    $$PWD/xcompositeeglintegration.cpp
