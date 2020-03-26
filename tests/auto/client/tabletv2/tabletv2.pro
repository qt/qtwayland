include (../shared/shared.pri)

WAYLANDSERVERSOURCES += \
    $$PWD/../../../../src/3rdparty/protocol/tablet-unstable-v2.xml

TARGET = tst_tabletv2
SOURCES += tst_tabletv2.cpp
