include (../shared/shared.pri)

WAYLANDSERVERSOURCES += \
    $$PWD/../../../../src/3rdparty/protocol/xdg-output-unstable-v1.xml

TARGET = tst_xdgoutput
SOURCES += tst_xdgoutput.cpp

