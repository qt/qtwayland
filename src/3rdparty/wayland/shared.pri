INCLUDEPATH += $$PWD

WAYLAND_INSTALL_DIR = $$(WAYLAND_INSTALL_DIR)
isEmpty(WAYLAND_INSTALL_DIR) {
    DESTDIR=$$PWD/../../../lib/
} else {
    DESTDIR=$$WAYLAND_INSTALL_DIR
}

use_pkgconfig {
   CONFIG += link_pkgconfig
   PKGCONFIG += libffi
} else {
    LIBS += -L $$PWD/../../../../lib/ -lffi 
    INCLUDEPATH += $$PWD/../ffi
}
