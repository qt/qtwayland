WAYLANDSOURCES += \
                $$PWD/../../../extensions/surface-extension.xml \
                $$PWD/../../../extensions/sub-surface-extension.xml \
                $$PWD/../../../extensions/output-extension.xml \
                $$PWD/../../../extensions/touch-extension.xml


HEADERS += \
    $$PWD/wlcompositor.h \
    $$PWD/wldisplay.h \
    $$PWD/wloutput.h \
    $$PWD/wlshmbuffer.h \
    $$PWD/wlsurface.h \
    $$PWD/wlshellsurface.h \
    $$PWD/wlinputdevice.h \
    $$PWD/wldatadevicemanager.h \
    $$PWD/wldatadevice.h \
    $$PWD/wldataoffer.h \
    $$PWD/wldatasource.h \
    $$PWD/wlextendedsurface.h \
    $$PWD/wlextendedoutput.h \
    $$PWD/wlsubsurface.h \
    $$PWD/wltouch.h \
    $$PWD/../../shared/qwaylandmimehelper.h \
    $$PWD/wlsurfacebuffer.h \
    $$PWD/wlregion.h

SOURCES += \
    $$PWD/wlcompositor.cpp \
    $$PWD/wldisplay.cpp \
    $$PWD/wloutput.cpp \
    $$PWD/wlshmbuffer.cpp \
    $$PWD/wlsurface.cpp \
    $$PWD/wlshellsurface.cpp \
    $$PWD/wlinputdevice.cpp \
    $$PWD/wldatadevicemanager.cpp \
    $$PWD/wldatadevice.cpp \
    $$PWD/wldataoffer.cpp \
    $$PWD/wldatasource.cpp \
    $$PWD/wlextendedsurface.cpp \
    $$PWD/wlextendedoutput.cpp \
    $$PWD/wlsubsurface.cpp \
    $$PWD/wltouch.cpp \
    $$PWD/../../shared/qwaylandmimehelper.cpp \
    $$PWD/wlsurfacebuffer.cpp \
    $$PWD/wlregion.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../../shared

