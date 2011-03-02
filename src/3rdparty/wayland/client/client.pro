TEMPLATE = lib
TARGET = wayland-client
DESTDIR=$$PWD/../../../../lib/

CONFIG -= qt
CONFIG += shared

INCLUDEPATH += $$PWD/.. \
               $$PWD/../../ffi

LIBS += -L $$PWD/../../../../lib/ -lffi

SOURCES = ../wayland-client.c \
          ../wayland-protocol.c \
          ../connection.c \
          ../wayland-util.c \
          ../wayland-hash.c

OBJECTS_DIR = .obj
