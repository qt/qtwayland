TEMPLATE=subdirs
include($$OUT_PWD/client/qtwaylandclient-config.pri)
include($$OUT_PWD/compositor/qtwaylandcompositor-config.pri)
QT_FOR_CONFIG += waylandclient-private waylandcompositor-private

qtConfig(wayland-client)|qtConfig(wayland-server) {
    sub_qtwaylandscanner.subdir = qtwaylandscanner
    sub_qtwaylandscanner.target = sub-qtwaylandscanner
    SUBDIRS += sub_qtwaylandscanner

    qtConfig(wayland-client) {
        sub_client.subdir = client
        sub_client.depends = sub-qtwaylandscanner
        sub_client.target = sub-client
        SUBDIRS += sub_client
    }

    sub_plugins.subdir = plugins
    sub_plugins.depends += sub-qtwaylandscanner
    qtConfig(wayland-client):sub_plugins.depends += sub-client
    qtConfig(wayland-server):sub_plugins.depends += sub-compositor
    sub_plugins.target = sub-plugins
    SUBDIRS += sub_plugins

    qtConfig(wayland-server) {
        sub_compositor.subdir = compositor
        sub_compositor.depends = sub-qtwaylandscanner
        sub_compositor.target = sub-compositor
        SUBDIRS += sub_compositor
    }

    sub_imports.subdir = imports
    qtConfig(wayland-client):sub_imports.depends += sub-client
    qtConfig(wayland-server):sub_imports.depends += sub-compositor
    sub_imports.target = sub-imports
    SUBDIRS += sub_imports
}
