TEMPLATE = lib
TARGET = wayland-server

CONFIG -= qt
CONFIG += shared
CONFIG += use_pkgconfig

include(../shared.pri)

SOURCES = ../event-loop.c \
          ../wayland-server.c \
          ../wayland-protocol.c \
          ../connection.c \
          ../wayland-util.c \
          ../wayland-hash.c \
          ../wayland-shm.c

OBJECTS_DIR = .obj
