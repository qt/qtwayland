TEMPLATE = subdirs
QT_FOR_CONFIG += waylandclient-private

qtConfig(wayland-client-fullscreen-shell-v1): SUBDIRS += fullscreen-shell-v1
qtConfig(wayland-client-ivi-shell): SUBDIRS += ivi-shell
qtConfig(wayland-client-wl-shell): SUBDIRS += wl-shell
qtConfig(wayland-client-xdg-shell): SUBDIRS += xdg-shell
qtConfig(wayland-client-xdg-shell-v5): SUBDIRS += xdg-shell-v5
qtConfig(wayland-client-xdg-shell-v6): SUBDIRS += xdg-shell-v6
