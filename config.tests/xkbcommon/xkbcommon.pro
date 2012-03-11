TEMPLATE = app
TARGET = xkbcommon
QT = core
DEPENDPATH += .
INCLUDEPATH += .

!contains(QT_CONFIG, no-pkg-config) {
    QMAKE_CFLAGS_XKBCOMMON=$$system(pkg-config --cflags xkbcommon 2>/dev/null)
    QMAKE_LIBS_XKBCOMMON=$$system(pkg-config --libs xkbcommon 2>/dev/null)
}

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_XKBCOMMON
QMAKE_CFLAGS += $$QMAKE_CFLAGS_XKBCOMMON
LIBS += $$QMAKE_LIBS_XKBCOMMON

# Input
SOURCES += main.cpp
