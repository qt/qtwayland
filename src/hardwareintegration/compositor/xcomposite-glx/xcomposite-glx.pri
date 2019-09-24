include($$PWD/../xcomposite_share/xcomposite_share.pri)

QMAKE_USE_PRIVATE += wayland-server

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/xcompositeglxintegration.h

SOURCES += \
    $$PWD/xcompositeglxintegration.cpp
