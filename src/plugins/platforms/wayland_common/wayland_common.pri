#This file(wayland_common.pri) is included from .pro files of GL integrations.

include ($$PWD/wayland_common_share.pri)

INCLUDEPATH += $$PWD

staticlib = $$shadowed($$PWD)/$${QMAKE_PREFIX_STATICLIB}wayland_common.$${QMAKE_EXTENSION_STATICLIB}
LIBS += $$staticlib
PRE_TARGETDEPS += $$staticlib

