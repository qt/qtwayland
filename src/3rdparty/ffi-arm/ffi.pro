TEMPLATE = lib
TARGET = ffi
DESTDIR=$$PWD/../../../lib/

CONFIG -= qt
CONFIG += shared

SOURCES =  ffi.c \
          prep_cif.c \
          types.c \
          sysv.S

OBJECTS_DIR = .obj
