TEMPLATE = lib
TARGET = wayland-server
DESTDIR=$$PWD/../../../../lib/

CONFIG -= qt
CONFIG += shared

INCLUDEPATH += $$PWD/.. \
               $$PWD/../../ffi

LIBS += -L $$PWD/../../../../lib/ -lffi

SOURCES = ../event-loop.c \
          ../wayland-server.c \
          ../wayland-protocol.c \
          ../connection.c \
          ../wayland-util.c \
          ../wayland-hash.c

OBJECTS_DIR = .obj
