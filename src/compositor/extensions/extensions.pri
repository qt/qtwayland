CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../extensions/sub-surface-extension.xml \
    ../extensions/surface-extension.xml \
    ../extensions/touch-extension.xml \
    ../extensions/qtkey-extension.xml \
    ../extensions/windowmanager.xml \

HEADERS += \
    extensions/qwlextendedsurface_p.h \
    extensions/qwlsubsurface_p.h \
    extensions/qwlqttouch_p.h \
    extensions/qwlqtkey_p.h \
    extensions/qwlshellsurface_p.h \
    extensions/qwaylandwindowmanagerextension.h \
    extensions/qwaylandwindowmanagerextension_p.h \
    extensions/qwltextinput_p.h \
    extensions/qwltextinputmanager_p.h \
    extensions/qwlinputpanel_p.h \
    extensions/qwlinputpanelsurface_p.h \

SOURCES += \
    extensions/qwlextendedsurface.cpp \
    extensions/qwlsubsurface.cpp \
    extensions/qwlqttouch.cpp \
    extensions/qwlqtkey.cpp \
    extensions/qwlshellsurface.cpp \
    extensions/qwaylandwindowmanagerextension.cpp \
    extensions/qwltextinput.cpp \
    extensions/qwltextinputmanager.cpp \
    extensions/qwlinputpanel.cpp \
    extensions/qwlinputpanelsurface.cpp \

INCLUDEPATH += extensions
