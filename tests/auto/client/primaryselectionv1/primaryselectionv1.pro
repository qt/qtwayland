include (../shared/shared.pri)

WAYLANDSERVERSOURCES += \
    $$PWD/../../../../src/3rdparty/protocol/wp-primary-selection-unstable-v1.xml

TARGET = tst_primaryselectionv1
SOURCES += tst_primaryselectionv1.cpp
