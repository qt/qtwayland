TEMPLATE = lib
CONFIG += staticlib

include (qt-compositor.pri)

installPath = $$INSTALLBASE

target.path = $$installPath/lib
headers_path = $$installPath/include

headers.path = $$headers_path/qt-compositor
headers.files = $$HEADERS

INSTALLS = target headers
