TEMPLATE=subdirs
QT_FOR_CONFIG += waylandclient-private

qtHaveModule(waylandclient): \
    SUBDIRS += client

qtHaveModule(waylandclient):qtHaveModule(waylandcompositor): \
    SUBDIRS += cmake

qtHaveModule(waylandcompositor): \
    SUBDIRS += compositor
