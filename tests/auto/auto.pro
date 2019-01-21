TEMPLATE=subdirs
QT_FOR_CONFIG += waylandclient-private

qtConfig(wayland-client): \
    SUBDIRS += client

qtConfig(wayland-client):qtHaveModule(waylandcompositor): \
    SUBDIRS += cmake

qtHaveModule(waylandcompositor): \
    SUBDIRS += compositor
