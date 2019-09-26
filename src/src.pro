TEMPLATE=subdirs
include($$OUT_PWD/client/qtwaylandclient-config.pri)
include($$OUT_PWD/compositor/qtwaylandcompositor-config.pri)
QT_FOR_CONFIG += waylandclient-private waylandcompositor-private

qtConfig(wayland-client)|qtConfig(wayland-server) {
    SUBDIRS += qtwaylandscanner

    qtConfig(wayland-client) {
        client.depends = qtwaylandscanner
        SUBDIRS += client
    }

    plugins.depends += qtwaylandscanner
    qtConfig(wayland-client):plugins.depends += client
    qtConfig(wayland-server):plugins.depends += compositor
    SUBDIRS += plugins

    qtConfig(wayland-server) {
        compositor.depends = qtwaylandscanner
        SUBDIRS += compositor
    }

    qtConfig(wayland-client):imports.depends += client
    qtConfig(wayland-server):imports.depends += compositor
    SUBDIRS += imports
}
