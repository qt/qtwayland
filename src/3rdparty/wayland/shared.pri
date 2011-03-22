INCLUDEPATH += $$PWD

use_pkgconfig {
   CONFIG += link_pkgconfig
   PKGCONFIG += libffi
} else {
    LIBS += -L $$PWD/../../../../lib/ -lffi 
    INCLUDEPATH += $$PWD/../ffi
}
