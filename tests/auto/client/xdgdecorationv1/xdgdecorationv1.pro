include (../shared/shared.pri)

WAYLANDSERVERSOURCES += \
    $$PWD/../../../../src/3rdparty/protocol/xdg-decoration-unstable-v1.xml

TARGET = tst_xdgdecorationv1
SOURCES += tst_xdgdecorationv1.cpp
