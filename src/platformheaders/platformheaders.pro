TEMPLATE = subdirs
VERSION = $$MODULE_VERSION
MODULE_INCNAME = QtPlatformHeaders

include(waylandfunctions/waylandfunctions.pri)

load(qt_module_headers)
#load(qt_docs)
load(qt_installs)

