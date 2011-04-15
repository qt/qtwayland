include (../xcomposite_share/xcomposite_share.pri)

LIBS += -lXcomposite -lX11

HEADERS += \
    $$PWD/xcompositeglxintegration.h

SOURCES += \
    $$PWD/xcompositeglxintegration.cpp
