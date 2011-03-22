TEMPLATE = lib
TARGET = wayland-client
DESTDIR=$$PWD/../../../../lib/

CONFIG -= qt
CONFIG += shared
CONFIG += use_pkgconfig

include(../shared.pri)

SOURCES = ../wayland-client.c \
          ../wayland-protocol.c \
          ../connection.c \
          ../wayland-util.c \
          ../wayland-hash.c

OBJECTS_DIR = .obj
