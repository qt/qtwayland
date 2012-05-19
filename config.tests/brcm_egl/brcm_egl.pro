TEMPLATE = app
TARGET = brcm_egl
QT = core gui
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -lEGL -lGLESv2

# Input
SOURCES += main.cpp
